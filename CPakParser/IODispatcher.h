#pragma once

#include "IOStore.h"

class FIoDispatcher final : FNoncopyable
{
public:
	static void Mount(std::shared_ptr<FFileIoStore> Backend);

	//class FIoBatch NewBatch();
	//TIoStatusOr<FIoMappedRegion> OpenMapped(const FIoChunkId& ChunkId, const FIoReadOptions& Options);

	// Polling methods
	//CORE_API bool					DoesChunkExist(const FIoChunkId& ChunkId) const;
	//CORE_API TIoStatusOr<uint64>	GetSizeForChunk(const FIoChunkId& ChunkId) const;
	//CORE_API int64					GetTotalLoaded() const;

	static bool IsInitialized();
	static void Initialize();
	static void InitializePostSettings();
	static void Shutdown();

	static FIoDispatcher& Get()
	{
		static FIoDispatcher Instance;
		return Instance;
	}

private:
	FIoDispatcher();

	friend class FIoRequest;
	friend class FIoBatch;
	friend class FIoQueue;

	std::vector<std::shared_ptr<FFileIoStore>> Backends;
	bool bIsInitialized = false;

	/*std::shared_ptr<FIoDispatcherBackendContext> BackendContext;
	
	FIoRequestAllocator* RequestAllocator = nullptr;
	FBatchAllocator BatchAllocator;
	FRunnableThread* Thread = nullptr;
	FEvent* DispatcherEvent = nullptr;
	FCriticalSection WaitingLock;
	FIoRequestImpl* WaitingRequestsHead = nullptr;
	FIoRequestImpl* WaitingRequestsTail = nullptr;
	FCriticalSection UpdateLock;
	TArray<FIoRequestImpl*> RequestsToCancel;
	TArray<FIoRequestImpl*> RequestsToReprioritize;
	TAtomic<bool> bStopRequested{ false };
	FIoDispatcher::FIoContainerUnmountedEvent ContainerUnmountedEvent;
	uint64 PendingIoRequestsCount = 0;
	int64 TotalLoaded = 0;
	FIoRequestStats RequestStats;
	*/

};

std::shared_ptr<FFileIoStore> CreateIoDispatcherFileBackend();