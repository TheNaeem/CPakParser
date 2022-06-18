#pragma once

class FFileIoStoreReader 
{
public:
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