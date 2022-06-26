#pragma once

#include "IOStore.h"

class FFileIoStoreReader 
{
public:
	FFileIoStoreReader() = default;
	~FFileIoStoreReader();

	ReadStatus Initialize(const char* InTocFilePath, int32_t Order);

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
	FIoContainerHeader ReadContainerHeader() const;
	void ReopenAllFileHandles();

private:
	const FIoOffsetAndLength* FindChunkInternal(const FIoChunkId& ChunkId) const;
	uint64_t GetTocAllocatedSize() const;

	struct FPerfectHashMap
	{
		std::vector<int32_t> TocChunkHashSeeds;
		std::vector<FIoChunkId> TocChunkIds;
		std::vector<FIoOffsetAndLength> TocOffsetAndLengths;
	};

	FPerfectHashMap PerfectHashMap;
	std::unordered_map<FIoChunkId, FIoOffsetAndLength, FIoChunkId> TocImperfectHashMapFallback;
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
	FIoContainerHeader Mount(const char* InTocPath, int32_t Order, FGuid EncryptionKeyGuid, FAESKey EncryptionKey);
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