#include "CoreTypes.h"
#include "IOStore.h"
#include "IOStoreReader.h"
#include "BinaryReader.h"
#include "MemoryUtil.h"
#include "MemoryReader.h"
#include <sstream>
#include <filesystem>
#include <windows.h>

std::atomic_uint32_t FFileIoStoreReader::GlobalPartitionIndex{ 0 };
std::atomic_uint32_t FFileIoStoreReader::GlobalContainerInstanceId{ 0 };

static constexpr bool bPerfectHashingEnabled = true;

FFileIoStoreReader::~FFileIoStoreReader()
{
	for (auto& Partition : ContainerFile.Partitions)
	{
		CloseHandle(Partition.FileHandle);
	}

	PerfectHashMap.TocChunkHashSeeds.clear();
	PerfectHashMap.TocChunkIds.clear();
	PerfectHashMap.TocOffsetAndLengths.clear();
	TocImperfectHashMapFallback.clear();
	ContainerFile = FFileIoStoreContainerFile();
	ContainerId = FIoContainerId();
	Order = INDEX_NONE;
}

bool OpenContainer(const char* ContainerFilePath, HANDLE& ContainerFileHandle, uint64_t& ContainerFileSize)
{
	auto FileSize = std::filesystem::file_size(ContainerFilePath);

	if (FileSize < 0)
	{
		return false;
	}

	auto FileHandle = CreateFileA(
		ContainerFilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (!FileHandle)
	{
		return false;
	}

	ContainerFileHandle = FileHandle;
	ContainerFileSize = FileSize;

	return true;
}

ReadStatus FFileIoStoreReader::Initialize(const char* InTocFilePath, int32_t InOrder)
{
	std::string_view ContainerPathView(InTocFilePath);

	if (!ContainerPathView.ends_with(".utoc"))
	{
		return ReadStatus(EIoErrorCode::FileOpenFailed, std::string(ContainerPathView) + " was not a .utoc file.");
	}

	auto BasePathView = ContainerPathView.substr(0, ContainerPathView.length() - 5);
	ContainerFile.FilePath = BasePathView;

	FIoStoreTocResource TocResource;
	auto Status = FIoStoreTocResource::Read(InTocFilePath, EIoStoreTocReadOptions::Default, TocResource);

	if (!Status.IsOk()) return Status;

	ContainerFile.PartitionSize = TocResource.Header.PartitionSize;
	ContainerFile.Partitions.resize(TocResource.Header.PartitionCount);

	for (auto PartitionIndex = 0; PartitionIndex < TocResource.Header.PartitionCount; ++PartitionIndex)
	{
		FFileIoStoreContainerFilePartition& Partition = ContainerFile.Partitions[PartitionIndex];

		std::stringstream ContainerFilePath;
		ContainerFilePath << BasePathView;

		if (PartitionIndex > 0)
		{
			ContainerFilePath << "_s" << PartitionIndex;
		}

		ContainerFilePath << ".ucas";
		Partition.FilePath = ContainerFilePath.str();

		ContainerFilePath.flush();

		if (!OpenContainer(Partition.FilePath.c_str(), Partition.FileHandle, Partition.FileSize))
		{
			return ReadStatus(EIoErrorCode::FileOpenFailed, "Failed to open IoStore container file " + Partition.FilePath);
		}

		Partition.ContainerFileIndex = GlobalPartitionIndex++;
	}

	if (bPerfectHashingEnabled && !TocResource.ChunkPerfectHashSeeds.empty())
	{
		for (auto ChunkIndexWithoutPerfectHash : TocResource.ChunkIndicesWithoutPerfectHash)
		{
			auto ChunkId = TocResource.ChunkIds[ChunkIndexWithoutPerfectHash];
			auto ChunkOffset = TocResource.ChunkOffsetLengths[ChunkIndexWithoutPerfectHash];

			TocImperfectHashMapFallback.insert({ ChunkId, ChunkOffset });
		}

		PerfectHashMap.TocChunkHashSeeds = TocResource.ChunkPerfectHashSeeds;
		PerfectHashMap.TocOffsetAndLengths = TocResource.ChunkOffsetLengths;
		PerfectHashMap.TocChunkIds = TocResource.ChunkIds;

		bHasPerfectHashMap = true;
	}
	else
	{
		for (auto ChunkIndex = 0; ChunkIndex < TocResource.Header.TocEntryCount; ++ChunkIndex)
			TocImperfectHashMapFallback.insert({ TocResource.ChunkIds[ChunkIndex], TocResource.ChunkOffsetLengths[ChunkIndex] });

		bHasPerfectHashMap = false;
	}

	ContainerFile.CompressionMethods = TocResource.CompressionMethods;
	ContainerFile.CompressionBlockSize = TocResource.Header.CompressionBlockSize;
	ContainerFile.CompressionBlocks = TocResource.CompressionBlocks;
	ContainerFile.ContainerFlags = TocResource.Header.ContainerFlags;
	ContainerFile.EncryptionKeyGuid = TocResource.Header.EncryptionKeyGuid;
	ContainerFile.ContainerInstanceId = ++GlobalContainerInstanceId;

	ContainerId = TocResource.Header.ContainerId;
	Order = InOrder;

	return ReadStatus::Ok;
}

FIoContainerHeader FFileIoStoreReader::ReadContainerHeader() const
{
	auto HeaderChunkId = CreateIoChunkId(ContainerId.Value(), 0, EIoChunkType::ContainerHeader);
	auto OffsetAndLength = FindChunkInternal(HeaderChunkId);

	if (!OffsetAndLength)
	{
		ReadStatus(EIoErrorCode::NotFound, "Container header chunk not found");
	}

	auto CompressionBlockSize = ContainerFile.CompressionBlockSize;
	auto Offset = OffsetAndLength->GetOffset();
	auto Size = OffsetAndLength->GetLength();
	auto RequestEndOffset = Offset + Size;
	auto RequestBeginBlockIndex = int32_t(Offset / CompressionBlockSize);
	auto RequestEndBlockIndex = int32_t((RequestEndOffset - 1) / CompressionBlockSize);

	auto CompressionBlockEntry = &ContainerFile.CompressionBlocks[RequestBeginBlockIndex];
	auto PartitionIndex = int32_t(CompressionBlockEntry->GetOffset() / ContainerFile.PartitionSize);
	auto RawOffset = CompressionBlockEntry->GetOffset() % ContainerFile.PartitionSize;
	auto& Partition = ContainerFile.Partitions[PartitionIndex];

	FIoBuffer IoBuffer(MemoryUtil::Align(Size, FAESKey::AESBlockSize));
	auto ContainerFileHandle = std::make_unique<BinaryReader>(Partition.FilePath.c_str());

	if (!ContainerFileHandle || !ContainerFileHandle->IsValid())
		ReadStatus(EIoErrorCode::FileOpenFailed, "Failed to open container file for TOC"); //expect a crash if this happens 
	
	ContainerFileHandle->Seek(RawOffset);
	ContainerFileHandle->Read(IoBuffer.Data(), IoBuffer.DataSize());

	const bool bSigned = EnumHasAnyFlags(ContainerFile.ContainerFlags, EIoContainerFlags::Signed);
	const bool bEncrypted = ContainerFile.EncryptionKey.IsValid();

	if (bSigned || bEncrypted)
	{
		auto BlockData = IoBuffer.Data();

		for (auto CompressedBlockIndex = RequestBeginBlockIndex; CompressedBlockIndex <= RequestEndBlockIndex; ++CompressedBlockIndex)
		{
			CompressionBlockEntry = &ContainerFile.CompressionBlocks[CompressedBlockIndex];

			const auto BlockSize = MemoryUtil::Align(CompressionBlockEntry->GetCompressedSize(), FAESKey::AESBlockSize);

			if (bEncrypted)
			{
				ContainerFile.EncryptionKey.DecryptData(BlockData, uint32_t(BlockSize));
			}

			BlockData += BlockSize;
		}
	}

	FMemoryReaderView Ar(std::span<uint8_t>{ IoBuffer.Data(), IoBuffer.DataSize() });
	FIoContainerHeader ContainerHeader;
	Ar << ContainerHeader;

	return ContainerHeader;
}

const FIoOffsetAndLength* FFileIoStoreReader::FindChunkInternal(const FIoChunkId& ChunkId) const
{
	if (!bHasPerfectHashMap)
		return &TocImperfectHashMapFallback.find(ChunkId)->second;

	uint32_t ChunkCount = PerfectHashMap.TocChunkIds.size();
	if (!ChunkCount) return nullptr;

	uint32_t SeedCount = PerfectHashMap.TocChunkHashSeeds.size();
	uint32_t SeedIndex = FIoStoreTocResource::HashChunkIdWithSeed(0, ChunkId) % SeedCount;

	int32_t Seed = PerfectHashMap.TocChunkHashSeeds[SeedIndex];
	if (!Seed) return nullptr;

	uint32_t Slot;
	if (Seed < 0)
	{
		auto SeedAsIndex = static_cast<uint32_t>(-Seed - 1);
		if (SeedAsIndex < ChunkCount)
		{
			Slot = static_cast<uint32_t>(SeedAsIndex);
		}
		else return &TocImperfectHashMapFallback.find(ChunkId)->second;
	}
	else Slot = FIoStoreTocResource::HashChunkIdWithSeed(static_cast<uint32_t>(Seed), ChunkId) % ChunkCount;

	return &PerfectHashMap.TocOffsetAndLengths[Slot];
}