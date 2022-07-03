#pragma once

#include "IOStore.h"

class FFileIoStoreReader 
{
public:
	FFileIoStoreReader() = default;
	~FFileIoStoreReader();

	ReadStatus Initialize(const char* InTocFilePath);

	uint32_t GetContainerInstanceId() const
	{
		return ContainerFile.ContainerInstanceId;
	}

	ReadStatus Close();
	bool DoesChunkExist(const FIoChunkId& ChunkId) const;
	uint64_t GetSizeForChunk(const FIoChunkId& ChunkId) const;
	const FIoOffsetAndLength* Resolve(const FIoChunkId& ChunkId) const;
	const FFileIoStoreContainerFile& GetContainerFile() const { return ContainerFile; }
	IMappedFileHandle* GetMappedContainerFileHandle(uint64_t TocOffset);
	const FIoContainerId& GetContainerId() const { return ContainerId; }
	int32_t GetOrder() const { return Order; }
	bool IsEncrypted() const { return EnumHasAnyFlags(ContainerFile.ContainerFlags, EIoContainerFlags::Encrypted); }
	bool IsSigned() const { return EnumHasAnyFlags(ContainerFile.ContainerFlags, EIoContainerFlags::Signed); }
	const FGuid& GetEncryptionKeyGuid() const { return ContainerFile.EncryptionKeyGuid; }
	void SetEncryptionKey(const FAESKey& Key) { ContainerFile.EncryptionKey = Key; }
	const FAESKey& GetEncryptionKey() const { return ContainerFile.EncryptionKey; }
	FIoContainerHeader ReadContainerHeader();
	void ReopenAllFileHandles();

private:
	FIoOffsetAndLength FindChunkInternal(FIoChunkId& ChunkId);
	uint64_t GetTocAllocatedSize() const;

	struct FPerfectHashMap
	{
		std::vector<int32_t> TocChunkHashSeeds;
		std::vector<FIoChunkId> TocChunkIds;
		std::vector<FIoOffsetAndLength> TocOffsetAndLengths;
	};

	FPerfectHashMap PerfectHashMap;
	phmap::flat_hash_map<FIoChunkId, FIoOffsetAndLength> TocImperfectHashMapFallback;
	FFileIoStoreContainerFile ContainerFile;
	FIoContainerId ContainerId;
	int32_t Order;
	bool bClosed = false;
	bool bHasPerfectHashMap = false;

	static std::atomic_uint32_t GlobalPartitionIndex;
	static std::atomic_uint32_t GlobalContainerInstanceId;
};

class FFileIoStore final
{
public:
	FIoContainerHeader Mount(std::string InTocPath, FGuid EncryptionKeyGuid, FAESKey EncryptionKey);
	void Initialize();

private:
	uint64_t ReadBufferSize = 0;

	mutable std::shared_mutex IoStoreReadersLock;
	std::vector<std::unique_ptr<class FFileIoStoreReader>> IoStoreReaders;

	/*
	std::shared_ptr<const FIoDispatcherBackendContext> BackendContext;
	FFileIoStoreStats Stats;
	FFileIoStoreBlockCache BlockCache;
	FFileIoStoreBufferAllocator BufferAllocator;
	FFileIoStoreRequestAllocator RequestAllocator;
	FFileIoStoreRequestQueue RequestQueue;
	FFileIoStoreRequestTracker RequestTracker;
	TUniquePtr<IPlatformFileIoStore> PlatformImpl;
	FRunnableThread* Thread = nullptr;
	bool bIsMultithreaded;
	std::atomic_bool bStopRequested{ false };
	FFileIoStoreCompressionContext* FirstFreeCompressionContext = nullptr;
	FFileIoStoreCompressedBlock* ReadyForDecompressionHead = nullptr;
	FFileIoStoreCompressedBlock* ReadyForDecompressionTail = nullptr;
	FCriticalSection DecompressedBlocksCritical;
	FFileIoStoreCompressedBlock* FirstDecompressedBlock = nullptr;
	FIoRequestImpl* CompletedRequestsHead = nullptr;
	FIoRequestImpl* CompletedRequestsTail = nullptr;
	*/
};