#include "IOStoreReader.h"
#include "MemoryReader.h"
#include "IoDirectoryIndex.h"
#include <sstream>
#include <windows.h>
#include "FileReader.h"

std::atomic_uint32_t FFileIoStore::GlobalPartitionIndex{ 0 };
std::atomic_uint32_t FFileIoStore::GlobalContainerInstanceId{ 0 };

static constexpr bool bPerfectHashingEnabled = true;

bool FFileIoStoreContainerFilePartition::OpenContainer(const char* ContainerFilePath)
{
	FileSize = std::filesystem::file_size(ContainerFilePath);

	if (FileSize < 0)
	{
		return false;
	}

	Ar = std::make_unique<FFileReader>(ContainerFilePath);

	return true;
}

void FFileIoStore::Initialize()
{
	ReadBufferSize = (GIoDispatcherBufferSizeKB > 0 ? uint64_t(GIoDispatcherBufferSizeKB) << 10 : 256 << 10);
}

FIoContainerHeader FFileIoStore::Mount(std::string InTocPath, FGuid EncryptionKeyGuid, FAESKey EncryptionKey)
{
	Log<Success>("Mounting TOC: " + InTocPath);

	auto Reader = std::make_shared<FIoStoreReader>(InTocPath.c_str());
	Reader->Initialize(true);

	if (Reader->IsEncrypted() && FEncryptionKeyManager::HasKey(EncryptionKeyGuid))
	{
		Reader->SetEncryptionKey(EncryptionKey);
	}

	return Reader->ReadContainerHeader();
}

FIoOffsetAndLength FIoStoreReader::FindChunkInternal(FIoChunkId& ChunkId)
{
	if (!bHasPerfectHashMap)
		return TocImperfectHashMapFallback[ChunkId];

	auto ChunkCount = uint32_t(Container.TocResource->ChunkIds.size());

	if (!ChunkCount) return FIoOffsetAndLength();

	auto SeedCount = Container.TocResource->ChunkPerfectHashSeeds.size();
	auto SeedIndex = FIoStoreTocResource::HashChunkIdWithSeed(0, ChunkId) % SeedCount;

	int32_t Seed = Container.TocResource->ChunkPerfectHashSeeds[SeedIndex];
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

	if (Container.TocResource->ChunkIds[Slot] != ChunkId)
		return FIoOffsetAndLength();

	return Container.TocResource->ChunkOffsetLengths[Slot];
}

FIoContainerHeader FIoStoreReader::ReadContainerHeader()
{
	auto TocRsrc = Container.TocResource;

	auto HeaderChunkId = FIoChunkId(TocRsrc->Header.ContainerId.Value(), 0, EIoChunkType::ContainerHeader);
	auto OffsetAndLength = FindChunkInternal(HeaderChunkId);

	if (!OffsetAndLength.IsValid())
	{
		Log<Error>("Container header chunk not found: " + this->Container.FilePath);
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

	auto BufSize = Align(Size, FAESKey::AESBlockSize);

	auto IoBuffer = std::make_unique<uint8_t[]>(BufSize);

	{
		auto ContainerFileHandle = FFileReader(Container.Partitions[PartitionIndex].FilePath.c_str());

		if (!ContainerFileHandle.IsValid())
			Log<Error>("Failed to open container file for TOC"); //expect a crash if this happens 

		ContainerFileHandle.Seek(RawOffset);
		ContainerFileHandle.Serialize(IoBuffer.get(), BufSize);
	}

	const bool bSigned = EnumHasAnyFlags(TocRsrc->Header.ContainerFlags, EIoContainerFlags::Signed);
	const bool bEncrypted = Container.EncryptionKey.IsValid();

	if (bEncrypted)
	{
		uint8_t* BlockData = IoBuffer.get();

		for (auto CompressedBlockIndex = RequestBeginBlockIndex; CompressedBlockIndex <= RequestEndBlockIndex; ++CompressedBlockIndex)
		{
			CompressionBlockEntry = &TocRsrc->CompressionBlocks[CompressedBlockIndex];

			const auto BlockSize = Align(CompressionBlockEntry->GetCompressedSize(), FAESKey::AESBlockSize);

			Container.EncryptionKey.DecryptData(BlockData, uint32_t(BlockSize));

			BlockData += BlockSize;
		}
	}

	FMemoryReaderView Ar(std::span<uint8_t>{ IoBuffer.get(), BufSize });
	FIoContainerHeader ContainerHeader;
	Ar << ContainerHeader;

	return Container.Header = ContainerHeader;
}

FIoStoreTocChunkInfo FIoStoreReader::CreateTocChunkInfo(uint32_t TocEntryIndex)
{
	auto TocResource = Toc->GetResource();

	auto& OffsetLength = TocResource->ChunkOffsetLengths[TocEntryIndex]; // TODO: come back and see how much of this data we actually need

	FIoStoreTocChunkInfo Ret(TocEntryIndex);
	Ret.Offset = OffsetLength.GetOffset();
	Ret.Size = OffsetLength.GetLength();
	Ret.SetOwningFile(Toc);

	return Ret;
}

FIoStoreReader::FIoStoreReader(const char* ContainerPath)
{
	Toc = std::make_shared<FIoStoreToc>(
		FIoStoreTocResource(ContainerPath, EIoStoreTocReadOptions::ReadAll));

	auto TocRsrc = Toc->GetResource();

	std::string_view ContainerPathView(ContainerPath);

	if (!ContainerPathView.ends_with(".utoc"))
	{
		Log<Error>(std::string(ContainerPathView) + " was not a .utoc file.");
		return;
	}

	auto BasePathView = ContainerPathView.substr(0, ContainerPathView.length() - 5);

	Container.TocResource = TocRsrc;
	Container.FilePath = BasePathView;
	Container.Partitions.resize(TocRsrc->Header.PartitionCount);

	for (uint32_t PartitionIndex = 0; PartitionIndex < TocRsrc->Header.PartitionCount; ++PartitionIndex)
	{
		FFileIoStoreContainerFilePartition& Partition = Container.Partitions[PartitionIndex];

		std::stringstream ContainerFilePath;
		ContainerFilePath << BasePathView;

		if (PartitionIndex > 0)
		{
			ContainerFilePath << "_s" << PartitionIndex;
		}

		ContainerFilePath << ".ucas";
		Partition.FilePath = ContainerFilePath.str();

		ContainerFilePath.flush();

		if (!Partition.OpenContainer(Partition.FilePath.c_str()))
		{
			Log<Error>("Failed to open IoStore container file " + Partition.FilePath);
			return;
		}

		Partition.ContainerFileIndex = FFileIoStore::GlobalPartitionIndex++;
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
		for (uint32_t ChunkIndex = 0; ChunkIndex < TocRsrc->Header.TocEntryCount; ++ChunkIndex)
			TocImperfectHashMapFallback.insert_or_assign(TocRsrc->ChunkIds[ChunkIndex], TocRsrc->ChunkOffsetLengths[ChunkIndex]);

		bHasPerfectHashMap = false;
	}

	Container.ContainerInstanceId = ++FFileIoStore::GlobalContainerInstanceId;
}

void FIoStoreReader::Read(int32_t InPartitionIndex, int64_t Offset, int64_t Len, uint8_t* OutBuffer)
{
	SCOPE_LOCK(Lock);

	Container.Partitions[InPartitionIndex].Ar->Seek(Offset);
	Container.Partitions[InPartitionIndex].Ar->Serialize(OutBuffer, Len);
}

void FIoStoreReader::ParseDirectoryIndex(FIoDirectoryIndexResource& DirectoryIndex, std::string& Path, uint32_t DirectoryIndexHandle)
{
	static constexpr uint32_t InvalidHandle = ~uint32_t(0);

	uint32_t File = DirectoryIndex.DirectoryEntries[DirectoryIndexHandle].FirstFileEntry;

	while (File != InvalidHandle)
	{
		auto& FileEntry = DirectoryIndex.FileEntries[File];
		auto& FileName = DirectoryIndex.StringTable[FileEntry.Name];
		auto FullDir = DirectoryIndex.MountPoint + Path;

		if (FileName.ends_with('\0'))
			FileName.pop_back();

		FGameFileManager::AddFile(FullDir, FileName, CreateTocChunkInfo(FileEntry.UserData));

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

void FIoStoreReader::Initialize(bool bSerializeDirectoryIndex)
{
	Toc->SetReader(shared_from_this());
	auto TocResource = Toc->GetResource();

	if (bSerializeDirectoryIndex &&
		EnumHasAnyFlags(TocResource->Header.ContainerFlags, EIoContainerFlags::Indexed) &&
		TocResource->DirectoryIndexBuffer.size() > 0)
	{
		if (TocResource->Header.IsEncrypted() && FEncryptionKeyManager::HasKey(TocResource->Header.EncryptionKeyGuid))
		{
			Toc->GetEncryptionKey().DecryptData(TocResource->DirectoryIndexBuffer.data(), uint32_t(TocResource->DirectoryIndexBuffer.size()));
		}

		FMemoryReaderView DirectoryIndexReader(TocResource->DirectoryIndexBuffer);

		FIoDirectoryIndexResource DirectoryIndex;
		DirectoryIndexReader << DirectoryIndex;

		if (!DirectoryIndex.DirectoryEntries.size()) return;

		DirectoryIndex.MountPoint.pop_back();

		std::string _ = "";
		this->ParseDirectoryIndex(DirectoryIndex, _);
	}

	Ar = std::make_unique<FFileReader>(TocResource->TocPath.string().c_str());
}