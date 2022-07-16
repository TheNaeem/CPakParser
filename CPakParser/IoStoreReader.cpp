#include "GameFileManager.h"
#include "IOStore.h"
#include "IOStoreReader.h"
#include "BinaryReader.h"
#include "MemoryUtil.h"
#include "MemoryReader.h"
#include "IoDirectoryIndex.h"
#include <sstream>
#include <windows.h>

std::atomic_uint32_t FFileIoStore::Reader::GlobalPartitionIndex{ 0 };
std::atomic_uint32_t FFileIoStore::Reader::GlobalContainerInstanceId{ 0 };

static constexpr bool bPerfectHashingEnabled = true;

__forceinline void FIoStoreToc::Initialize()
{
	FEncryptionKeyManager::GetKey(Toc->Header.EncryptionKeyGuid, Key);
}

FFileIoStore::Reader::~Reader()
{
	for (auto& Partition : ContainerFile.Partitions)
	{
		CloseHandle(Partition.FileHandle);
	}

	TocImperfectHashMapFallback.clear();
	ContainerFile = FFileIoStoreContainerFile();
}

bool OpenContainer(const char* ContainerFilePath, HANDLE& ContainerFileHandle, uint64_t& ContainerFileSize)
{
	auto FileSize = std::filesystem::file_size(ContainerFilePath);

	/*if (FileSize < 0)
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

	ContainerFileHandle = FileHandle;*/

	ContainerFileSize = FileSize;

	return true;
}

FFileIoStore::Reader::Reader(const char* InTocFilePath)
{
	std::string_view ContainerPathView(InTocFilePath);

	if (!ContainerPathView.ends_with(".utoc"))
	{
		ReadStatus(ReadErrorCode::FileOpenFailed, std::string(ContainerPathView) + " was not a .utoc file.");
		return;
	}

	auto BasePathView = ContainerPathView.substr(0, ContainerPathView.length() - 5);
	ContainerFile.FilePath = BasePathView;

	auto TocRsrc = ContainerFile.TocResource = std::make_shared<FIoStoreTocResource>(InTocFilePath, EIoStoreTocReadOptions::ReadAll);

	ContainerFile.Partitions.resize(TocRsrc->Header.PartitionCount);

	for (auto PartitionIndex = 0; PartitionIndex < TocRsrc->Header.PartitionCount; ++PartitionIndex)
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
			ReadStatus(ReadErrorCode::FileOpenFailed, "Failed to open IoStore container file " + Partition.FilePath);
			return;
		}

		Partition.ContainerFileIndex = GlobalPartitionIndex++;
	}

	if (bPerfectHashingEnabled && !TocRsrc->ChunkPerfectHashSeeds.empty())
	{
		for (auto ChunkIndexWithoutPerfectHash : TocRsrc->ChunkIndicesWithoutPerfectHash)
		{
			TocImperfectHashMapFallback.insert_or_assign(
				TocRsrc->ChunkIds[ChunkIndexWithoutPerfectHash],
				TocRsrc->ChunkOffsetLengths[ChunkIndexWithoutPerfectHash]);
		}

		bHasPerfectHashMap = true;
	}
	else
	{
		for (auto ChunkIndex = 0; ChunkIndex < TocRsrc->Header.TocEntryCount; ++ChunkIndex)
			TocImperfectHashMapFallback.insert_or_assign(TocRsrc->ChunkIds[ChunkIndex], TocRsrc->ChunkOffsetLengths[ChunkIndex]);

		bHasPerfectHashMap = false;
	}

	ContainerFile.ContainerInstanceId = ++GlobalContainerInstanceId;
}

FIoContainerHeader FFileIoStore::Reader::ReadContainerHeader()
{
	auto TocRsrc = ContainerFile.TocResource;

	auto HeaderChunkId = FIoChunkId(TocRsrc->Header.ContainerId.Value(), 0, EIoChunkType::ContainerHeader);
	auto OffsetAndLength = FindChunkInternal(HeaderChunkId);

	if (!OffsetAndLength.IsValid())
	{
		ReadStatus(ReadErrorCode::NotFound, "Container header chunk not found: " + this->ContainerFile.FilePath);
		return FIoContainerHeader();
	}

	auto CompressionBlockSize = TocRsrc->Header.CompressionBlockSize;
	auto Offset = OffsetAndLength.GetOffset();
	auto Size = OffsetAndLength.GetLength();
	auto RequestEndOffset = Offset + Size;
	auto RequestBeginBlockIndex = int32_t(Offset / CompressionBlockSize);
	auto RequestEndBlockIndex = int32_t((RequestEndOffset - 1) / CompressionBlockSize);

	auto CompressionBlockEntry = &TocRsrc->CompressionBlocks[RequestBeginBlockIndex];
	auto PartitionIndex = int32_t(CompressionBlockEntry->GetOffset() / TocRsrc->Header.PartitionSize);
	auto RawOffset = CompressionBlockEntry->GetOffset() % TocRsrc->Header.PartitionSize;

	auto BufSize = MemoryUtil::Align(Size, FAESKey::AESBlockSize);

	auto IoBuffer = std::make_unique<uint8_t[]>(BufSize);
	auto ContainerFileHandle = std::make_unique<BinaryReader>(ContainerFile.Partitions[PartitionIndex].FilePath.c_str());

	if (!ContainerFileHandle || !ContainerFileHandle->IsValid())
		ReadStatus(ReadErrorCode::FileOpenFailed, "Failed to open container file for TOC"); //expect a crash if this happens 

	ContainerFileHandle->Seek(RawOffset);
	ContainerFileHandle->Read(IoBuffer.get(), BufSize);

	const bool bSigned = EnumHasAnyFlags(TocRsrc->Header.ContainerFlags, EIoContainerFlags::Signed);
	const bool bEncrypted = ContainerFile.EncryptionKey.IsValid();

	if (bEncrypted)
	{
		uint8_t* BlockData = IoBuffer.get();

		for (auto CompressedBlockIndex = RequestBeginBlockIndex; CompressedBlockIndex <= RequestEndBlockIndex; ++CompressedBlockIndex)
		{
			CompressionBlockEntry = &TocRsrc->CompressionBlocks[CompressedBlockIndex];

			const auto BlockSize = MemoryUtil::Align(CompressionBlockEntry->GetCompressedSize(), FAESKey::AESBlockSize);

			ContainerFile.EncryptionKey.DecryptData(BlockData, uint32_t(BlockSize));

			BlockData += BlockSize;
		}
	}

	FMemoryReaderView Ar(std::span<uint8_t>{ IoBuffer.get(), BufSize });
	FIoContainerHeader ContainerHeader;
	Ar << ContainerHeader;

	return ContainerHeader;
}

FIoOffsetAndLength FFileIoStore::Reader::FindChunkInternal(FIoChunkId& ChunkId)
{
	if (!bHasPerfectHashMap)
		return TocImperfectHashMapFallback[ChunkId];

	auto ChunkCount = uint32_t(ContainerFile.TocResource->ChunkIds.size());

	if (!ChunkCount) return FIoOffsetAndLength();

	uint32_t SeedCount = ContainerFile.TocResource->ChunkPerfectHashSeeds.size();
	uint32_t SeedIndex = FIoStoreTocResource::HashChunkIdWithSeed(0, ChunkId) % SeedCount;

	int32_t Seed = ContainerFile.TocResource->ChunkPerfectHashSeeds[SeedIndex];
	if (!Seed) return FIoOffsetAndLength();

	uint32_t Slot;
	if (Seed < 0)
	{
		auto SeedAsIndex = static_cast<uint32_t>(-Seed - 1);
		if (SeedAsIndex < ChunkCount)
		{
			Slot = static_cast<uint32_t>(SeedAsIndex);
		}
		else return TocImperfectHashMapFallback[ChunkId];
	}
	else Slot = FIoStoreTocResource::HashChunkIdWithSeed(static_cast<uint32_t>(Seed), ChunkId) % ChunkCount;

	if (ContainerFile.TocResource->ChunkIds[Slot] != ChunkId)
		return FIoOffsetAndLength();

	return ContainerFile.TocResource->ChunkOffsetLengths[Slot];
}

void ParseDirectoryIndex(FIoDirectoryIndexResource& DirectoryIndex, std::string& Path, uint32_t DirectoryIndexHandle = 0)
{
	static constexpr uint32_t InvalidHandle = ~uint32_t(0);

	uint32_t File = DirectoryIndex.DirectoryEntries[DirectoryIndexHandle].FirstFileEntry;

	while (File != InvalidHandle)
	{
		auto& FileEntry = DirectoryIndex.FileEntries[File];
		auto& FileName = DirectoryIndex.StringTable[FileEntry.Name];

		FGameFileManager::AddFile(Path, FileName);

		File = FileEntry.NextFileEntry;
	}

	auto Child = DirectoryIndex.DirectoryEntries[DirectoryIndexHandle].FirstChildEntry;

	while (Child != InvalidHandle)
	{
		auto& Entry = DirectoryIndex.DirectoryEntries[Child];

		auto ChildDirPath = Path + DirectoryIndex.StringTable[Entry.Name];
		ChildDirPath[ChildDirPath.size() - 1] = '/'; //replace null terminator with path divider 

		uint32_t File = DirectoryIndex.DirectoryEntries[DirectoryIndexHandle].FirstFileEntry;

		ParseDirectoryIndex(DirectoryIndex, ChildDirPath, Child);

		Child = Entry.NextSiblingEntry;
	}
}

void FIoStoreReader::Initialize(std::shared_ptr<FIoStoreTocResource> TocResource)
{
	Toc = TocResource;

	if (EnumHasAnyFlags(TocResource->Header.ContainerFlags, EIoContainerFlags::Indexed) &&
		TocResource->DirectoryIndexBuffer.size() > 0)
	{
		if (TocResource->Header.IsEncrypted() && FEncryptionKeyManager::HasKey(TocResource->Header.EncryptionKeyGuid))
		{
			Toc.GetEncryptionKey().DecryptData(TocResource->DirectoryIndexBuffer.data(), TocResource->DirectoryIndexBuffer.size());
		}

		FMemoryReaderView Ar(TocResource->DirectoryIndexBuffer);

		FIoDirectoryIndexResource DirectoryIndex;
		Ar << DirectoryIndex;

		if (!DirectoryIndex.DirectoryEntries.size()) return;

		std::string _ = "";
		ParseDirectoryIndex(DirectoryIndex, _);
	}
}