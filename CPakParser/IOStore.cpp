#include "CoreTypes.h"
#include "IODispatcher.h"
#include "BinaryReader.h"
#include "AES.h"
#include "IOStoreReader.h"

const ReadStatus ReadStatus::Ok{ EIoErrorCode::Ok, "OK" };
const ReadStatus ReadStatus::Unknown{ EIoErrorCode::Unknown, "Unknown Status" };
const ReadStatus ReadStatus::Invalid{ EIoErrorCode::InvalidCode, "Invalid Code" };

ReadStatus FIoStoreTocResource::Read(const char* TocFilePath, EIoStoreTocReadOptions ReadOptions, FIoStoreTocResource& OutTocResource)
{
	PakBinaryReader TocFileReader(TocFilePath);

	if (!TocFileReader.IsValid())
		return ReadStatus(EIoErrorCode::FileOpenFailed, "Failed to open IoStore TOC file");
	
	FIoStoreTocHeader& Header = OutTocResource.Header = TocFileReader.Read<FIoStoreTocHeader>();

	if (!Header.CheckMagic())
		return ReadStatus(EIoErrorCode::CorruptToc, "Could not read TOC file magic");
	
	if (Header.TocHeaderSize != sizeof(FIoStoreTocHeader))
		ReadStatus(EIoErrorCode::CorruptToc, "User defined FIoStoreTocHeader is not the same size as the one used in the TOC");
	

	if (Header.TocCompressedBlockEntrySize != sizeof(FIoStoreTocCompressedBlockEntry)) 
		ReadStatus(EIoErrorCode::CorruptToc, "User defined FIoStoreTocCompressedBlockEntry is not the same size as the one used in the TOC");

	auto TotalTocSize = TocFileReader.Size() - sizeof(FIoStoreTocHeader);
	auto TocMetaSize = Header.TocEntryCount * sizeof(FIoStoreTocEntryMeta);
	auto DefaultTocSize = TotalTocSize - Header.DirectoryIndexSize - TocMetaSize;
	auto TocSize = DefaultTocSize;

	if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadTocMeta))
	{
		TocSize = TotalTocSize;
	}
	else if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadDirectoryIndex))
	{
		TocSize = DefaultTocSize + Header.DirectoryIndexSize;
	}

	auto TocBuffer = std::make_unique<char[]>(TocSize);
	TocFileReader.Read(TocBuffer.get(), TocSize);

	char* DataPtr = TocBuffer.get();

	OutTocResource.ChunkIds = std::span<FIoChunkId>{ (FIoChunkId*)DataPtr, Header.TocEntryCount };
	DataPtr += Header.TocEntryCount * sizeof FIoChunkId;

	OutTocResource.ChunkOffsetLengths = std::span<FIoOffsetAndLength>{ (FIoOffsetAndLength*)DataPtr, Header.TocEntryCount };
	DataPtr += Header.TocEntryCount * sizeof(FIoOffsetAndLength);

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
		OutTocResource.ChunkPerfectHashSeeds = std::span<int32_t>{ (int32_t*)DataPtr, PerfectHashSeedsCount };
		DataPtr += PerfectHashSeedsCount * sizeof(int32_t);
	}
	if (ChunksWithoutPerfectHashCount)
	{
		OutTocResource.ChunkIndicesWithoutPerfectHash = std::span<int32_t>{ (int32_t*)DataPtr, ChunksWithoutPerfectHashCount };
		DataPtr += ChunksWithoutPerfectHashCount * sizeof(int32_t);
	}

	OutTocResource.CompressionBlocks = std::span<FIoStoreTocCompressedBlockEntry>{ (FIoStoreTocCompressedBlockEntry*)DataPtr, Header.TocCompressedBlockEntryCount };
	DataPtr += Header.TocCompressedBlockEntryCount * sizeof(FIoStoreTocCompressedBlockEntry);

	OutTocResource.CompressionMethods.reserve(Header.CompressionMethodNameCount + 1);
	OutTocResource.CompressionMethods.push_back("");

	for (auto CompressonNameIndex = 0; CompressonNameIndex < Header.CompressionMethodNameCount; CompressonNameIndex++)
	{
		const char* AnsiCompressionMethodName = DataPtr + CompressonNameIndex * Header.CompressionMethodNameLength;
		OutTocResource.CompressionMethods.push_back(AnsiCompressionMethodName);
	}

	DataPtr += Header.CompressionMethodNameCount * Header.CompressionMethodNameLength;

	auto DirectoryIndexBuffer = (uint8_t*)DataPtr;
	bool bIsSigned = EnumHasAnyFlags(Header.ContainerFlags, EIoContainerFlags::Signed);
	if (bIsSigned)
	{
		auto HashSize = *(uint32_t*)(DataPtr);
		DirectoryIndexBuffer = (uint8_t*)(DataPtr + 1 + HashSize + HashSize + Header.TocCompressedBlockEntryCount);

		//TODO: get chunk block signatures
	}

	if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadDirectoryIndex) &&
		EnumHasAnyFlags(Header.ContainerFlags, EIoContainerFlags::Indexed) &&
		Header.DirectoryIndexSize > 0)
	{
		OutTocResource.DirectoryIndexBuffer = std::span<uint8_t>{ DirectoryIndexBuffer, Header.DirectoryIndexSize };
	}

	auto TocMeta = DirectoryIndexBuffer + Header.DirectoryIndexSize;
	if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadTocMeta))
	{
		OutTocResource.ChunkMetas = std::span<FIoStoreTocEntryMeta>{ (FIoStoreTocEntryMeta*)TocMeta, Header.TocEntryCount };
	}

	if (Header.Version < static_cast<uint8_t>(EIoStoreTocVersion::PartitionSize))
	{
		Header.PartitionCount = 1;
		Header.PartitionSize = MAX_uint64;
	}

	return ReadStatus::Ok;
}

uint64_t FIoStoreTocResource::HashChunkIdWithSeed(int32_t Seed, const FIoChunkId& ChunkId)
{
	auto Data = ChunkId.GetData();
	auto DataSize = ChunkId.GetSize();
	auto Hash = Seed ? static_cast<uint64_t>(Seed) : 0xcbf29ce484222325;

	for (auto Index = 0; Index < DataSize; ++Index)
	{
		Hash = (Hash * 0x00000100000001B3) ^ Data[Index];
	}

	return Hash;
}

FIoDispatcher::FIoDispatcher()
{
}

void FFileIoStore::Initialize()
{
	ReadBufferSize = (GIoDispatcherBufferSizeKB > 0 ? uint64_t(GIoDispatcherBufferSizeKB) << 10 : 256 << 10);
}

void FIoDispatcher::Initialize()
{
	if (Get().bIsInitialized)
	{
		return;
	}

	Get().bIsInitialized = true;

	if (!Get().Backends.empty())
	{
		for (std::shared_ptr<FFileIoStore>& Backend : Get().Backends)
		{
			Backend->Initialize();
		}
	}
}

std::shared_ptr<FFileIoStore> CreateIoDispatcherFileBackend()
{
	return std::make_shared<FFileIoStore>();
}

void FIoDispatcher::Mount(std::shared_ptr<FFileIoStore> Backend)
{
	Get().Backends.push_back(Backend);

	if (!Get().bIsInitialized) return;

	Backend->Initialize();
}

FIoContainerHeader FFileIoStore::Mount(const char* InTocPath, int32_t Order, FGuid EncryptionKeyGuid, FAESKey EncryptionKey)
{
	auto Reader = std::make_unique<FFileIoStoreReader>();
	Reader->Initialize(InTocPath, Order);

	if (Reader->IsEncrypted() && 
		Reader->GetEncryptionKeyGuid() == EncryptionKeyGuid && 
		EncryptionKey.IsValid())
	{
		Reader->SetEncryptionKey(EncryptionKey);
	}

	auto ContainerHeader = Reader->ReadContainerHeader();
}