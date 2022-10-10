#include "IOStoreReader.h"

#include "MemoryReader.h"
#include "IoDirectoryIndex.h"
#include <sstream>
#include <windows.h>
#include "FileReader.h"
#include "GameFileManager.h"
#include "Compression.h"
#include "GlobalContext.h"

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

FIoStoreReader::FIoStoreReader(const char* ContainerPath, std::atomic_int32_t& PartitionIndex)
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

		Partition.ContainerFileIndex = PartitionIndex++;
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
}

TUniquePtr<uint8_t[]> FIoStoreReader::Read(FIoChunkId ChunkId) 
{
	auto OaL = Toc->GetOffsetAndLength(ChunkId);

	if (!OaL.IsValid())
		return nullptr;

	return Read(OaL);
}

TUniquePtr<uint8_t[]> FIoStoreReader::Read(FIoOffsetAndLength& OffsetAndLength) // TODO: make this async; look into StartAsyncRead
{
	auto TocResource = Toc->GetResource();

	auto Offset = OffsetAndLength.GetOffset();
	auto Len = OffsetAndLength.GetLength();
	auto CompressionBlockSize = TocResource->Header.CompressionBlockSize;

	auto FirstBlockIndex = Offset / CompressionBlockSize;
	auto LastBlockIndex = (Align(Offset + Len, CompressionBlockSize) - 1) / CompressionBlockSize;
	auto BlockCount = LastBlockIndex - FirstBlockIndex + 1;

	if (!BlockCount)
		return nullptr;

	auto& FirstBlock = TocResource->CompressionBlocks[FirstBlockIndex];
	auto& LastBlock = TocResource->CompressionBlocks[LastBlockIndex];

	auto PartitionIndex = static_cast<int32_t>(FirstBlock.GetOffset() / TocResource->Header.PartitionSize);

	auto ReadStartOffset = FirstBlock.GetOffset() % TocResource->Header.PartitionSize;
	auto ReadEndOffset = (LastBlock.GetOffset() + Align(LastBlock.GetCompressedSize(), FAESKey::AESBlockSize)) % TocResource->Header.PartitionSize;

	auto CompressedSize = ReadEndOffset - ReadStartOffset;

	auto DecompressionBuf = std::make_unique<uint8_t[]>(Len);
	auto CompressedBuf = std::make_unique<uint8_t[]>(CompressedSize);

	Read(PartitionIndex, ReadStartOffset, CompressedSize, CompressedBuf.get());

	uint64_t CompressedOffset = 0;
	uint64_t DecompressedOffset = 0;
	uint64_t OffsetInBlock = Offset % CompressionBlockSize;
	uint64_t RemainingSize = Len;

	for (auto i = FirstBlockIndex; i <= LastBlockIndex; ++i)
	{
		auto CompressedData = CompressedBuf.get() + CompressedOffset;
		auto DecompressedData = DecompressionBuf.get() + CompressedOffset;

		auto& CompressionBlock = TocResource->CompressionBlocks[i];

		auto BlockCompressedSize = Align(CompressionBlock.GetCompressedSize(), FAESKey::AESBlockSize);
		auto UncompressedSize = CompressionBlock.GetUncompressedSize();

		auto& CompressionMethod = TocResource->GetBlockCompressionMethod(CompressionBlock);

		if (TocResource->Header.IsEncrypted())
		{
			Toc->GetEncryptionKey().DecryptData(CompressedData, BlockCompressedSize);
		}

		if (CompressionMethod.empty())
		{
			memcpy(DecompressedData, CompressedData + OffsetInBlock, UncompressedSize - OffsetInBlock);
		}
		else if (OffsetInBlock || RemainingSize < UncompressedSize)
		{
			// TODO: is this really necessary? give this another look
			std::vector<uint8_t> TempBuffer(UncompressedSize);
			FCompression::DecompressMemory(CompressionMethod, TempBuffer.data(), UncompressedSize, CompressedData, CompressionBlock.GetCompressedSize());

			auto CopySize = std::min(UncompressedSize - OffsetInBlock, RemainingSize);
			memcpy(DecompressedData, TempBuffer.data() + OffsetInBlock, CopySize);
		}
		else
		{
			FCompression::DecompressMemory(CompressionMethod, DecompressedData, UncompressedSize, CompressedData, BlockCompressedSize);
		}

		CompressedOffset += BlockCompressedSize;
		DecompressedOffset += UncompressedSize;
		RemainingSize -= UncompressedSize;
		OffsetInBlock = 0;
	}

	return DecompressionBuf;
}

void FIoStoreReader::Read(int32_t InPartitionIndex, int64_t Offset, int64_t Len, uint8_t* OutBuffer)
{
	SCOPE_LOCK(Lock);

	Container.Partitions[InPartitionIndex].Ar->Seek(Offset);
	Container.Partitions[InPartitionIndex].Ar->Serialize(OutBuffer, Len);
}

void FIoStoreReader::ParseDirectoryIndex(FIoDirectoryIndexResource& DirectoryIndex, FGameFileManager& GameFiles, std::string& Path, uint32_t DirectoryIndexHandle)
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

		FFileEntryInfo Entry(FileEntry.UserData);
		Entry.SetOwningFile(Toc);

		GameFiles.AddFile(FullDir, FileName, Entry);

		File = FileEntry.NextFileEntry;
	}

	auto Child = DirectoryIndex.DirectoryEntries[DirectoryIndexHandle].FirstChildEntry;

	while (Child != InvalidHandle)
	{
		auto& Entry = DirectoryIndex.DirectoryEntries[Child];

		auto ChildDirPath = Path + DirectoryIndex.StringTable[Entry.Name];
		ChildDirPath[ChildDirPath.size() - 1] = '/'; //replace null terminator with path divider 

		uint32_t File = DirectoryIndex.DirectoryEntries[DirectoryIndexHandle].FirstFileEntry;

		ParseDirectoryIndex(DirectoryIndex, GameFiles, ChildDirPath, Child);

		Child = Entry.NextSiblingEntry;
	}
}

TSharedPtr<FIoStoreToc> FIoStoreReader::Initialize(TSharedPtr<GContext> Context, bool bSerializeDirectoryIndex)
{
	Toc->SetReader(shared_from_this());
	auto TocResource = Toc->GetResource();

	Ar = std::make_unique<FFileReader>(TocResource->TocPath.string().c_str());

	FAESKey Key;
	Context->EncryptionKeyManager.GetKey(TocResource->Header.EncryptionKeyGuid, Key);
	Toc->SetKey(Key);

	if (bSerializeDirectoryIndex &&
		EnumHasAnyFlags(TocResource->Header.ContainerFlags, EIoContainerFlags::Indexed) &&
		TocResource->DirectoryIndexBuffer.size() > 0)
	{
		if (TocResource->Header.IsEncrypted() && Context->EncryptionKeyManager.HasKey(TocResource->Header.EncryptionKeyGuid))
		{
			Toc->GetEncryptionKey().DecryptData(TocResource->DirectoryIndexBuffer.data(), uint32_t(TocResource->DirectoryIndexBuffer.size()));
		}

		FMemoryReaderView DirectoryIndexReader(TocResource->DirectoryIndexBuffer);

		FIoDirectoryIndexResource DirectoryIndex;
		DirectoryIndexReader << DirectoryIndex;

		if (!DirectoryIndex.DirectoryEntries.size())
		{
			Log<Warning>("Directory index has 0 files.");
			return nullptr;
		}

		DirectoryIndex.MountPoint.pop_back();

		Context->FilesManager.Reserve(DirectoryIndex.FileEntries.size());

		std::string _ = "";
		this->ParseDirectoryIndex(DirectoryIndex, Context->FilesManager, _);
	}

	return Toc;
}