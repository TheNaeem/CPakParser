#include "CoreTypes.h"
#include "BinaryReader.h"
#include "AES.h"
#include "IOStoreReader.h"

ReadStatus FIoStoreTocResource::Read(std::string TocFilePath, EIoStoreTocReadOptions ReadOptions, FIoStoreTocResource& OutTocResource)
{
	auto TocFileReader = std::make_unique<BinaryReader>(TocFilePath.c_str());

	if (!TocFileReader->IsValid())
		return ReadStatus(ReadErrorCode::FileOpenFailed, "Failed to open IoStore TOC file: " + TocFilePath);

	FIoStoreTocHeader& Header = OutTocResource.Header = TocFileReader->Read<FIoStoreTocHeader>();

	if (!Header.CheckMagic())
		return ReadStatus(ReadErrorCode::CorruptFile, "Could not read TOC file magic: " + TocFilePath);

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

	OutTocResource.ChunkIds = TocMemReader->ReadArray<FIoChunkId>(Header.TocEntryCount); 
	OutTocResource.ChunkOffsetLengths = TocMemReader->ReadArray<FIoOffsetAndLength>(Header.TocEntryCount);

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
		OutTocResource.ChunkPerfectHashSeeds = TocMemReader->ReadArray<int32_t>(PerfectHashSeedsCount);
	}

	if (ChunksWithoutPerfectHashCount)
	{
		OutTocResource.ChunkIndicesWithoutPerfectHash = TocMemReader->ReadArray<int32_t>(ChunksWithoutPerfectHashCount);
	}

	OutTocResource.CompressionBlocks = TocMemReader->ReadArray<FIoStoreTocCompressedBlockEntry>(Header.TocCompressedBlockEntryCount);

	OutTocResource.CompressionMethods.reserve(Header.CompressionMethodNameCount + 1);
	OutTocResource.CompressionMethods.push_back("");

	for (auto CompressonNameIndex = 0; CompressonNameIndex < Header.CompressionMethodNameCount; CompressonNameIndex++)
	{
		const char* AnsiCompressionMethodName = reinterpret_cast<char*>(TocMemReader->GetBuffer()) + CompressonNameIndex * Header.CompressionMethodNameLength;
		OutTocResource.CompressionMethods.push_back(AnsiCompressionMethodName);
	}

	TocMemReader->Seek(Header.CompressionMethodNameCount * Header.CompressionMethodNameLength);

	auto DirectoryIndexBuffer = std::make_unique<MemoryReader>(TocMemReader->GetBuffer());
	bool bIsSigned = EnumHasAnyFlags(Header.ContainerFlags, EIoContainerFlags::Signed);
	if (bIsSigned)
	{
		uint32_t HashSize = *(uint32_t*)(DirectoryIndexBuffer->GetBuffer());
		DirectoryIndexBuffer->Seek(1 + HashSize + HashSize + Header.TocCompressedBlockEntryCount);

		//TODO: get chunk block signatures
	}

	if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadDirectoryIndex) &&
		EnumHasAnyFlags(Header.ContainerFlags, EIoContainerFlags::Indexed) &&
		Header.DirectoryIndexSize > 0)
	{
		OutTocResource.DirectoryIndexBuffer = DirectoryIndexBuffer->ReadArray<uint8_t>(Header.DirectoryIndexSize);
	}

	DirectoryIndexBuffer->Seek(Header.DirectoryIndexSize);
	if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadTocMeta))
	{
		OutTocResource.ChunkMetas = DirectoryIndexBuffer->ReadArray<FIoStoreTocEntryMeta>(Header.TocEntryCount);
	}

	if (Header.Version < static_cast<uint8_t>(EIoStoreTocVersion::PartitionSize))
	{
		Header.PartitionCount = 1;
		Header.PartitionSize = MAX_uint64;
	}

	return ReadStatus(ReadErrorCode::Ok, "Successfully processed TOC header: " + TocFilePath);
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

FIoContainerHeader FFileIoStore::Mount(std::string InTocPath, FGuid EncryptionKeyGuid, FAESKey EncryptionKey)
{
	ReadStatus(ReadErrorCode::Ok, "Mounting TOC: " + InTocPath);

	auto Reader = std::make_unique<FFileIoStoreReader>();
	Reader->Initialize(InTocPath.c_str());

	if (Reader->IsEncrypted() &&
		Reader->GetEncryptionKeyGuid() == EncryptionKeyGuid &&
		EncryptionKey.IsValid())
	{
		Reader->SetEncryptionKey(EncryptionKey);
	}

	auto Ret = Reader->ReadContainerHeader();

	{
		WriteLock _(IoStoreReadersLock);

		IoStoreReaders.push_back(std::move(Reader));
	}

	return Ret;
}
