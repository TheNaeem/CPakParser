#include "IoStoreReader.h"

#include "Core/Globals/GlobalContext.h"
#include "Misc/Multithreading/Lock.h"
#include "../Toc/IoStoreToc.h"
#include <sstream>

import CPakParser.Files.FileEntry;
import CPakParser.IOStore.DirectoryIndex;
import CPakParser.Serialization.FileReader;
import CPakParser.Serialization.MemoryReader;
import CPakParser.Compression;
import CPakParser.IOStore.TocResource;
import CPakParser.Logging;

bool FIoStoreReader::IsEncrypted() const
{
	return Container.TocResource->Header.IsEncrypted();
}

static constexpr bool bPerfectHashingEnabled = true;

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

void FIoStoreReader::ReadContainerHeader()
{
	auto TocRsrc = Container.TocResource;

	auto HeaderChunkId = FIoChunkId(TocRsrc->Header.ContainerId.Value(), 0, EIoChunkType::ContainerHeader);
	auto OffsetAndLength = FindChunkInternal(HeaderChunkId);

	if (!OffsetAndLength.IsValid())
	{
		LogWarn("Container header chunk not found %s", this->Container.FilePath.c_str());
		return;
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
			LogError("Failed to open container file for TOC"); 

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

	FMemoryReader Ar(IoBuffer.get(), BufSize );
	Ar << Container.Header;
}

FIoStoreReader::FIoStoreReader(TSharedPtr<FIoStoreToc> InToc, std::atomic_int32_t& PartitionIndex)
{
	Toc = InToc;

	auto TocPtr = Toc.lock();

	if (!TocPtr)
	{
		LogError("Toc weak pointer passed into FIoStoreReader is invalid!");
		return;
	}

	auto TocRsrc = TocPtr->GetResource();

	std::string_view ContainerPathView(TocRsrc->TocPath);

	if (!ContainerPathView.ends_with(".utoc"))
	{
		LogError("%s was not a .utoc file", ContainerPathView.data());
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
			LogError("Failed to open IoStore container file %s", Partition.FilePath.c_str());
			return;
		}

		Partition.ContainerFileIndex = PartitionIndex;
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
	auto OaL = Toc.lock()->GetOffsetAndLength(ChunkId);

	if (!OaL.IsValid())
		return nullptr;

	return Read(OaL);
}

void FIoStoreReader::Read(uint64_t Offset, uint64_t Len, uint8_t* OutBuffer) // TODO: make this async; look into StartAsyncRead
{
	auto TocPtr = Toc.lock();
	auto TocResource = TocPtr->GetResource();

	auto CompressionBlockSize = TocResource->Header.CompressionBlockSize;

	auto FirstBlockIndex = Offset / CompressionBlockSize;
	auto LastBlockIndex = (Align(Offset + Len, CompressionBlockSize) - 1) / CompressionBlockSize;
	auto BlockCount = LastBlockIndex - FirstBlockIndex + 1;

	if (!BlockCount)
		return;

	auto& FirstBlock = TocResource->CompressionBlocks[FirstBlockIndex];
	auto& LastBlock = TocResource->CompressionBlocks[LastBlockIndex];

	auto PartitionIndex = static_cast<int32_t>(FirstBlock.GetOffset() / TocResource->Header.PartitionSize);

	auto ReadStartOffset = FirstBlock.GetOffset() % TocResource->Header.PartitionSize;
	auto ReadEndOffset = (LastBlock.GetOffset() + Align(LastBlock.GetCompressedSize(), FAESKey::AESBlockSize)) % TocResource->Header.PartitionSize;

	auto CompressedSize = ReadEndOffset - ReadStartOffset;

	auto CompressedBuf = std::make_unique<uint8_t[]>(CompressedSize);

	Read(PartitionIndex, ReadStartOffset, CompressedSize, CompressedBuf.get());

	uint64_t CompressedOffset = 0;
	uint64_t DecompressedOffset = 0;
	uint64_t OffsetInBlock = Offset % CompressionBlockSize;
	uint64_t RemainingSize = Len;

	for (auto i = FirstBlockIndex; i <= LastBlockIndex; ++i)
	{
		auto CompressedData = CompressedBuf.get() + CompressedOffset;
		auto DecompressedData = OutBuffer + CompressedOffset;

		auto& CompressionBlock = TocResource->CompressionBlocks[i];

		auto BlockCompressedSize = Align(CompressionBlock.GetCompressedSize(), FAESKey::AESBlockSize);
		auto UncompressedSize = CompressionBlock.GetUncompressedSize();

		auto& CompressionMethod = TocResource->GetBlockCompressionMethod(CompressionBlock);

		if (TocResource->Header.IsEncrypted())
		{
			TocPtr->GetEncryptionKey().DecryptData(CompressedData, BlockCompressedSize);
		}

		if (CompressionMethod.empty())
		{
			memcpyfst(DecompressedData, CompressedData + OffsetInBlock, UncompressedSize - OffsetInBlock);
		}
		else if (OffsetInBlock || RemainingSize < UncompressedSize)
		{
			// TODO: is this really necessary? give this another look
			std::vector<uint8_t> TempBuffer(UncompressedSize);
			FCompression::DecompressMemory(CompressionMethod, TempBuffer.data(), UncompressedSize, CompressedData, CompressionBlock.GetCompressedSize());

			auto CopySize = std::min(UncompressedSize - OffsetInBlock, RemainingSize);
			memcpyfst(DecompressedData, TempBuffer.data() + OffsetInBlock, CopySize);
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
}

void FIoStoreReader::Read(int32_t InPartitionIndex, int64_t Offset, int64_t Len, uint8_t* OutBuffer) // TODO: inline this
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
		Entry.SetOwningFile(Toc.lock().get());

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

void FIoStoreReader::Initialize(TSharedPtr<GContext> Context, bool bSerializeDirectoryIndex)
{
	auto TocPtr = Toc.lock();

	TocPtr->SetReader(shared_from_this());
	auto TocResource = TocPtr->GetResource();

	Ar = std::make_unique<FFileReader>(TocResource->TocPath.c_str());

	FAESKey Key;
	Context->EncryptionKeyManager.GetKey(TocResource->Header.EncryptionKeyGuid, Key);
	TocPtr->SetKey(Key);

	if (bSerializeDirectoryIndex &&
		EnumHasAnyFlags(TocResource->Header.ContainerFlags, EIoContainerFlags::Indexed) &&
		TocResource->DirectoryIndexBuffer.size() > 0)
	{
		if (TocResource->Header.IsEncrypted() && Context->EncryptionKeyManager.HasKey(TocResource->Header.EncryptionKeyGuid))
		{
			TocPtr->GetEncryptionKey().DecryptData(TocResource->DirectoryIndexBuffer.data(), uint32_t(TocResource->DirectoryIndexBuffer.size()));
		}

		FMemoryReader DirectoryIndexReader(TocResource->DirectoryIndexBuffer);

		FIoDirectoryIndexResource DirectoryIndex;
		DirectoryIndexReader << DirectoryIndex;

		if (!DirectoryIndex.DirectoryEntries.size())
		{
			LogWarn("Directory index has 0 files.");
			return;
		}

		DirectoryIndex.MountPoint.pop_back();

		Context->FilesManager.Reserve(DirectoryIndex.FileEntries.size());

		std::string _ = "";
		this->ParseDirectoryIndex(DirectoryIndex, Context->FilesManager, _);
	}
}