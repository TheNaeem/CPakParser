#pragma once

#include "MultiThreading.h"

class FArchive;

class FIoContainerId
{
public:
	inline FIoContainerId() = default;
	inline FIoContainerId(const FIoContainerId& Other) = default;
	inline FIoContainerId(FIoContainerId&& Other) = default;
	inline FIoContainerId& operator=(const FIoContainerId& Other) = default;

	static FIoContainerId FromName(const FName& Name);

	uint64_t Value() const
	{
		return Id;
	}

	inline bool IsValid() const
	{
		return Id != InvalidId;
	}

	inline bool operator<(FIoContainerId Other) const
	{
		return Id < Other.Id;
	}

	inline bool operator==(FIoContainerId Other) const
	{
		return Id == Other.Id;
	}

	inline bool operator!=(FIoContainerId Other) const
	{
		return Id != Other.Id;
	}

	inline friend uint32_t GetTypeHash(const FIoContainerId& In)
	{
		return uint32_t(In.Id);
	}

	friend FArchive& operator<<(FArchive& Ar, FIoContainerId& ContainerId);

private:
	inline explicit FIoContainerId(const uint64_t InId)
		: Id(InId) { }

	static constexpr uint64_t InvalidId = uint64_t(-1);

	uint64_t Id = InvalidId;
};

struct FIoContainerHeaderPackageRedirect
{
	FPackageId SourcePackageId;
	FPackageId TargetPackageId;
	FMappedName SourcePackageName;

	friend FArchive& operator<<(FArchive& Ar, FIoContainerHeaderPackageRedirect& PackageRedirect);
};

struct FIoContainerHeaderLocalizedPackage
{
	FPackageId SourcePackageId;
	FMappedName SourcePackageName;

	friend FArchive& operator<<(FArchive& Ar, FIoContainerHeaderLocalizedPackage& LocalizedPackage);
};

struct FIoContainerHeader
{
	enum
	{
		Signature = 0x496f436e
	};

	FIoContainerId ContainerId;
	std::vector<FPackageId> PackageIds;
	std::vector<uint8_t> StoreEntries; 
	std::vector<FPackageId> OptionalSegmentPackageIds;
	std::vector<uint8_t> OptionalSegmentStoreEntries; 
	std::vector<FDisplayNameEntryId> RedirectsNameMap;
	std::vector<FIoContainerHeaderLocalizedPackage> LocalizedPackages;
	std::vector<FIoContainerHeaderPackageRedirect> PackageRedirects;

	friend FArchive& operator<<(FArchive& Ar, FIoContainerHeader& ContainerHeader);
};

struct IIoDispatcherBackend
{
	/*
	virtual void Initialize(TSharedRef<const FIoDispatcherBackendContext> Context) = 0;
	virtual bool Resolve(FIoRequestImpl* Request) = 0;
	virtual void CancelIoRequest(FIoRequestImpl* Request) = 0;
	virtual void UpdatePriorityForIoRequest(FIoRequestImpl* Request) = 0;
	virtual bool DoesChunkExist(const FIoChunkId& ChunkId) const = 0;
	virtual TIoStatusOr<uint64> GetSizeForChunk(const FIoChunkId& ChunkId) const = 0;
	virtual FIoRequestImpl* GetCompletedRequests() = 0;
	virtual TIoStatusOr<FIoMappedRegion> OpenMapped(const FIoChunkId& ChunkId, const FIoReadOptions& Options) = 0;
	*/
};

class FFileIoStore final
	: public FRunnable
	, public IIoDispatcherBackend
{
	/*
public:
	FFileIoStore(std::unique_ptr<IPlatformFileIoStore>&& PlatformImpl);
	~FFileIoStore();

private:
	uint64_t ReadBufferSize = 0;
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
	mutable FRWLock IoStoreReadersLock;
	TArray<TUniquePtr<FFileIoStoreReader>> IoStoreReaders;
	FFileIoStoreCompressionContext* FirstFreeCompressionContext = nullptr;
	FFileIoStoreCompressedBlock* ReadyForDecompressionHead = nullptr;
	FFileIoStoreCompressedBlock* ReadyForDecompressionTail = nullptr;
	FCriticalSection DecompressedBlocksCritical;
	FFileIoStoreCompressedBlock* FirstDecompressedBlock = nullptr;
	FIoRequestImpl* CompletedRequestsHead = nullptr;
	FIoRequestImpl* CompletedRequestsTail = nullptr;
	*/
};