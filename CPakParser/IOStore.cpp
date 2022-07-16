#include "CoreTypes.h"
#include "BinaryReader.h"
#include "IOStoreReader.h"

FIoChunkId::FIoChunkId(uint64_t ChunkId, uint16_t ChunkIndex, EIoChunkType IoChunkType)
{
	*reinterpret_cast<uint64_t*>(&Id[0]) = ChunkId;
	*reinterpret_cast<uint16_t*>(&Id[8]) = NETWORK_ORDER16(ChunkIndex);
	*reinterpret_cast<uint8_t*>(&Id[11]) = static_cast<uint8_t>(IoChunkType);
}

void FFileIoStoreContainerFile::GetPartitionFileHandleAndOffset(uint64_t TocOffset, void*& OutFileHandle, uint64_t& OutOffset) 
{
	int32_t PartitionIndex = int32_t(TocOffset / TocResource->Header.PartitionSize);
	const FFileIoStoreContainerFilePartition& Partition = Partitions[PartitionIndex];
	OutFileHandle = Partition.FileHandle;
	OutOffset = TocOffset % TocResource->Header.PartitionSize;
}

FIoStoreTocResource::FIoStoreTocResource(std::string TocFilePath, EIoStoreTocReadOptions ReadOptions)
{
	auto TocFileReader = std::make_unique<BinaryReader>(TocFilePath.c_str());

	if (!TocFileReader->IsValid())
	{
		ReadStatus(ReadErrorCode::FileOpenFailed, "Failed to open IoStore TOC file: " + TocFilePath);
		return;
	}

	Header = TocFileReader->Read<FIoStoreTocHeader>();

	if (!Header.CheckMagic())
	{
		ReadStatus(ReadErrorCode::CorruptFile, "Could not read TOC file magic: " + TocFilePath);
		return;
	}

	if (Header.TocHeaderSize != sizeof(FIoStoreTocHeader))
		ReadStatus(ReadErrorCode::CorruptFile, "User defined FIoStoreTocHeader is not the same size as the one used in the TOC");


	if (Header.TocCompressedBlockEntrySize != sizeof(FIoStoreTocCompressedBlockEntry))
		ReadStatus(ReadErrorCode::CorruptFile, "User defined FIoStoreTocCompressedBlockEntry is not the same size as the one used in the TOC");

	uint64_t TotalTocSize = TocFileReader->Size() - sizeof(FIoStoreTocHeader);
	uint64_t TocMetaSize = Header.TocEntryCount * sizeof(FIoStoreTocEntryMeta);
	uint64_t DefaultTocSize = TotalTocSize - Header.DirectoryIndexSize - TocMetaSize;
	uint64_t TocSize = DefaultTocSize;

	if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadTocMeta))
	{
		TocSize = TotalTocSize;
	}
	else if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadDirectoryIndex))
	{
		TocSize = DefaultTocSize + Header.DirectoryIndexSize;
	}

	auto TocBuffer = std::make_unique<uint8_t[]>(TocSize);
	TocFileReader->Read(TocBuffer.get(), TocSize);

	auto TocMemReader = std::make_unique<MemoryReader>(TocBuffer.get());

	ChunkIds = TocMemReader->ReadArray<FIoChunkId>(Header.TocEntryCount);
	ChunkOffsetLengths = TocMemReader->ReadArray<FIoOffsetAndLength>(Header.TocEntryCount);

	uint32_t PerfectHashSeedsCount = 0;
	uint32_t ChunksWithoutPerfectHashCount = 0;

	if (Header.Version >= static_cast<uint8_t>(EIoStoreTocVersion::PerfectHashWithOverflow))
	{
		PerfectHashSeedsCount = Header.TocChunkPerfectHashSeedsCount;
		ChunksWithoutPerfectHashCount = Header.TocChunksWithoutPerfectHashCount;
	}
	else if (Header.Version >= static_cast<uint8_t>(EIoStoreTocVersion::PerfectHash))
	{
		PerfectHashSeedsCount = Header.TocChunkPerfectHashSeedsCount;
	}

	if (PerfectHashSeedsCount)
	{
		ChunkPerfectHashSeeds = TocMemReader->ReadArray<int32_t>(PerfectHashSeedsCount);
	}

	if (ChunksWithoutPerfectHashCount)
	{
		ChunkIndicesWithoutPerfectHash = TocMemReader->ReadArray<int32_t>(ChunksWithoutPerfectHashCount);
	}

	CompressionBlocks = TocMemReader->ReadArray<FIoStoreTocCompressedBlockEntry>(Header.TocCompressedBlockEntryCount);

	CompressionMethods.reserve(Header.CompressionMethodNameCount + 1);
	CompressionMethods.push_back({});

	for (auto CompressonNameIndex = 0; CompressonNameIndex < Header.CompressionMethodNameCount; CompressonNameIndex++)
	{
		const char* AnsiCompressionMethodName = reinterpret_cast<char*>(TocMemReader->GetBuffer()) + CompressonNameIndex * Header.CompressionMethodNameLength;
		CompressionMethods.push_back(AnsiCompressionMethodName);
	}

	TocMemReader->Seek(Header.CompressionMethodNameCount * Header.CompressionMethodNameLength);

	auto DirectoryIndexReader = std::make_unique<MemoryReader>(TocMemReader->GetBuffer());
	bool bIsSigned = EnumHasAnyFlags(Header.ContainerFlags, EIoContainerFlags::Signed);
	if (bIsSigned)
	{
		auto HashSize = DirectoryIndexReader->Read<uint32_t>();
		DirectoryIndexReader->Seek(HashSize + HashSize + (sizeof(FSHAHash) * Header.TocCompressedBlockEntryCount));

		//TODO: get chunk block signatures
	}

	if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadDirectoryIndex) &&
		EnumHasAnyFlags(Header.ContainerFlags, EIoContainerFlags::Indexed) &&
		Header.DirectoryIndexSize > 0)
	{
		auto Buf = DirectoryIndexReader->GetBuffer();

		this->DirectoryIndexBuffer = std::vector<uint8_t>(Buf, Buf + Header.DirectoryIndexSize);
	}

	DirectoryIndexReader->Seek(Header.DirectoryIndexSize);
	if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadTocMeta))
	{
		auto Buf = (FIoStoreTocEntryMeta*)DirectoryIndexReader->GetBuffer();

		ChunkMetas = std::vector<FIoStoreTocEntryMeta>(Buf, Buf + Header.TocEntryCount);
	}

	if (Header.Version < static_cast<uint8_t>(EIoStoreTocVersion::PartitionSize))
	{
		Header.PartitionCount = 1;
		Header.PartitionSize = MAX_uint64;
	}

	ReadStatus(ReadErrorCode::Ok, "Successfully processed TOC header: " + TocFilePath);
}

uint64_t FIoStoreTocResource::HashChunkIdWithSeed(int32_t Seed, const FIoChunkId& ChunkId)
{
	const uint8_t* Data = ChunkId.GetData();
	const uint32_t DataSize = ChunkId.GetSize();
	uint64_t Hash = Seed ? static_cast<uint64_t>(Seed) : 0xcbf29ce484222325;

	for (uint32_t Index = 0; Index < DataSize; ++Index)
	{
		Hash = (Hash * 0x00000100000001B3) ^ Data[Index];
	}

	return Hash;
}

void FFileIoStore::Initialize()
{
	ReadBufferSize = (GIoDispatcherBufferSizeKB > 0 ? uint64_t(GIoDispatcherBufferSizeKB) << 10 : 256 << 10);
}

std::shared_ptr<FFileIoStore> CreateIoDispatcherFileBackend()
{
	return std::make_shared<FFileIoStore>();
}

//TODO: refactor this, probably by removing FFileIoStoreReader and using FIoStoreReader
FIoContainerHeader FFileIoStore::Mount(std::string InTocPath, FGuid EncryptionKeyGuid, FAESKey EncryptionKey)
{
	ReadStatus(ReadErrorCode::Ok, "Mounting TOC: " + InTocPath);

	auto Reader = std::make_unique<FFileIoStore::Reader>(InTocPath.c_str());

	if (Reader->IsEncrypted() && FEncryptionKeyManager::HasKey(EncryptionKeyGuid))
	{
		Reader->SetEncryptionKey(EncryptionKey);
	}

	auto Ret = Reader->ReadContainerHeader();

	auto T = FIoStoreReader(Reader->GetTocResource());

	{
		WriteLock _(IoStoreReadersLock);

		IoStoreReaders.push_back(std::move(Reader));
	}

	return Ret;
}
