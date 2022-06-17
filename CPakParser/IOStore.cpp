#include "CoreTypes.h"
#include "IODispatcher.h"
#include "BinaryReader.h"
#include <sstream>
#include <filesystem>
#include <windows.h>

const FIoStatus FIoStatus::Ok{ EIoErrorCode::Ok, "OK" };
const FIoStatus FIoStatus::Unknown{ EIoErrorCode::Unknown, "Unknown Status" };
const FIoStatus FIoStatus::Invalid{ EIoErrorCode::InvalidCode, "Invalid Code" };
std::atomic_uint32_t FFileIoStoreReader::GlobalPartitionIndex{ 0 };
std::atomic_uint32_t FFileIoStoreReader::GlobalContainerInstanceId{ 0 };

uint32_t FIoBuffer::BufCore::AddRef() const
{
	return uint32_t(InterlockedIncrement(reinterpret_cast<long*>(&NumRefs)));
}

uint32_t FIoBuffer::BufCore::Release() const
{
	const int32_t Refs = InterlockedDecrement(reinterpret_cast<long*>(&NumRefs));

	if (Refs == 0)
	{
		delete this;
	}

	return uint32_t(Refs);
}

FIoStatus FIoStoreTocResource::Read(const char* TocFilePath, EIoStoreTocReadOptions ReadOptions, FIoStoreTocResource& OutTocResource)
{
	PakBinaryReader TocFileReader(TocFilePath);
	FIoStoreTocHeader& Header = OutTocResource.Header = TocFileReader.Read<FIoStoreTocHeader>();

	if (!Header.CheckMagic())
	{
		return FIoStatus(EIoErrorCode::CorruptToc, "Could not read TOC file magic");
	}

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

	return FIoStatus::Ok;
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

	/*
	uint64 BufferMemorySize = uint64(GIoDispatcherBufferMemoryMB) << 20ull;
	uint64 BufferSize = uint64(GIoDispatcherBufferSizeKB) << 10ull;
	uint32 BufferAlignment = uint32(GIoDispatcherBufferAlignment);
	BufferAllocator.Initialize(BufferMemorySize, BufferSize, BufferAlignment);

	uint64 CacheMemorySize = uint64(GIoDispatcherCacheSizeMB) << 20ull;
	BlockCache.Initialize(CacheMemorySize, BufferSize);

	PlatformImpl->Initialize({
		&BackendContext->WakeUpDispatcherThreadDelegate,
		&RequestAllocator,
		&BufferAllocator,
		&BlockCache,
		&Stats
	});

	uint64 DecompressionContextCount = uint64(GIoDispatcherDecompressionWorkerCount > 0 ? GIoDispatcherDecompressionWorkerCount : 4);
	for (uint64 ContextIndex = 0; ContextIndex < DecompressionContextCount; ++ContextIndex)
	{
		FFileIoStoreCompressionContext* Context = new FFileIoStoreCompressionContext();
		Context->Next = FirstFreeCompressionContext;
		FirstFreeCompressionContext = Context;
	}

	Thread = FRunnableThread::Create(this, TEXT("IoService"), 0, TPri_AboveNormal);*/
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

FIoChunkId CreateIoChunkId(uint64_t ChunkId, uint16_t ChunkIndex, EIoChunkType IoChunkType)
{
	uint8_t Data[12] = { 0 };

	*reinterpret_cast<uint64_t*>(&Data[0]) = ChunkId;
	*reinterpret_cast<uint16_t*>(&Data[8]) = NETWORK_ORDER16(ChunkIndex);
	*reinterpret_cast<uint8_t*>(&Data[11]) = static_cast<uint8_t>(IoChunkType);

	FIoChunkId IoChunkId;
	IoChunkId.Set(Data, 12);

	return IoChunkId;
}

FIoStatus FFileIoStoreReader::Initialize(const char* InTocFilePath, int32_t InOrder)
{
	std::string_view ContainerPathView(InTocFilePath);

	if (!ContainerPathView.ends_with(".utoc"))
	{
		return FIoStatus(EIoErrorCode::FileOpenFailed, std::string(ContainerPathView) + " was not a .utoc file.");
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
			return FIoStatus(EIoErrorCode::FileOpenFailed, "Failed to open IoStore container file " + Partition.FilePath);
		}

		Partition.ContainerFileIndex = GlobalPartitionIndex++;
	}

	if (!TocResource.ChunkPerfectHashSeeds.empty())
	{
		for (auto ChunkIndexWithoutPerfectHash : TocResource.ChunkIndicesWithoutPerfectHash)
		{
			auto ChunkId = TocResource.ChunkIds[ChunkIndexWithoutPerfectHash];
			auto ChunkOffset = TocResource.ChunkOffsetLengths[ChunkIndexWithoutPerfectHash];

			TocImperfectHashMapFallback.insert({ ChunkId, ChunkOffset });
		}

		PerfectHashMap.TocChunkHashSeeds.assign(TocResource.ChunkPerfectHashSeeds.begin(), TocResource.ChunkPerfectHashSeeds.end()); 
		PerfectHashMap.TocOffsetAndLengths.assign(TocResource.ChunkOffsetLengths.begin(), TocResource.ChunkOffsetLengths.end());
		PerfectHashMap.TocChunkIds.assign(TocResource.ChunkIds.begin(), TocResource.ChunkIds.end());
		bHasPerfectHashMap = true;
	}
	else
	{
		for (auto ChunkIndex = 0; ChunkIndex < TocResource.Header.TocEntryCount; ++ChunkIndex)
			TocImperfectHashMapFallback.insert({ TocResource.ChunkIds[ChunkIndex], TocResource.ChunkOffsetLengths[ChunkIndex] });
		
		bHasPerfectHashMap = false;
	}

	ContainerFile.CompressionMethods.assign(TocResource.CompressionMethods.begin(), TocResource.CompressionMethods.end());
	ContainerFile.CompressionBlockSize = TocResource.Header.CompressionBlockSize;
	ContainerFile.CompressionBlocks.assign(TocResource.CompressionBlocks.begin(), TocResource.CompressionBlocks.end());
	ContainerFile.ContainerFlags = TocResource.Header.ContainerFlags;
	ContainerFile.EncryptionKeyGuid = TocResource.Header.EncryptionKeyGuid;
	ContainerFile.ContainerInstanceId = ++GlobalContainerInstanceId;

	ContainerId = TocResource.Header.ContainerId;
	Order = InOrder;

	return FIoStatus::Ok;
}

const FIoOffsetAndLength* FFileIoStoreReader::FindChunkInternal(const FIoChunkId& ChunkId) const
{
	if (!bHasPerfectHashMap)
		return &TocImperfectHashMapFallback.find(ChunkId)->second;
	
	auto ChunkCount = PerfectHashMap.TocChunkIds.size();
	if (!ChunkCount) return nullptr;
	
	auto SeedCount = PerfectHashMap.TocChunkHashSeeds.size();
	auto SeedIndex = FIoStoreTocResource::HashChunkIdWithSeed(0, ChunkId) % SeedCount;

	auto Seed = PerfectHashMap.TocChunkHashSeeds[SeedIndex];
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
	
	if (PerfectHashMap.TocChunkIds[Slot] == ChunkId) 
		return &PerfectHashMap.TocOffsetAndLengths[Slot];
	
	return nullptr;
}

FIoContainerHeader FFileIoStoreReader::ReadContainerHeader() const
{
	auto HeaderChunkId = CreateIoChunkId(ContainerId.Value(), 0, EIoChunkType::ContainerHeader);
	auto OffsetAndLength = FindChunkInternal(HeaderChunkId);

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

	FIoBuffer IoBuffer(Size, 16);
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

	auto ContainerHeaderReadResult = Reader->ReadContainerHeader();
}