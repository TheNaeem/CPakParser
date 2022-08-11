#pragma once

#include "Archives.h"
#include <fstream>

struct FFileIoStoreContainerFilePartition
{
	FFileIoStoreContainerFilePartition() = default;
	FFileIoStoreContainerFilePartition(FFileIoStoreContainerFilePartition&&) = default;
	FFileIoStoreContainerFilePartition(const FFileIoStoreContainerFilePartition&) = delete;

	FFileIoStoreContainerFilePartition& operator=(FFileIoStoreContainerFilePartition&&) = default;
	FFileIoStoreContainerFilePartition& operator=(const FFileIoStoreContainerFilePartition&) = delete;

	bool OpenContainer(const char* ContainerFilePath);

	std::shared_ptr<std::ifstream> FileHandle;
	uint64_t FileSize = 0;
	uint32_t ContainerFileIndex = 0;
	std::string FilePath;
};

struct FFileIoStoreContainerFile
{
	FFileIoStoreContainerFile() = default;
	FFileIoStoreContainerFile(FFileIoStoreContainerFile&&) = default;
	FFileIoStoreContainerFile(const FFileIoStoreContainerFile&) = delete;

	FFileIoStoreContainerFile& operator=(FFileIoStoreContainerFile&&) = default;
	FFileIoStoreContainerFile& operator=(const FFileIoStoreContainerFile&) = delete;

	std::shared_ptr<struct FIoStoreTocResource> TocResource;
	std::string FilePath;
	FAESKey EncryptionKey;
	std::vector<class FSHAHash> BlockSignatureHashes;
	std::vector<FFileIoStoreContainerFilePartition> Partitions;
	uint32_t ContainerInstanceId = 0;

	void GetPartitionFileHandleAndOffset(uint64_t TocOffset, std::shared_ptr<std::ifstream>& OutFileHandle, uint64_t& OutOffset);
};

class FIoContainerId
{
public:
	inline FIoContainerId() : Id(InvalidId)
	{
	}

	inline FIoContainerId(const FIoContainerId& Other) = default;
	inline FIoContainerId(FIoContainerId&& Other) = default;
	inline FIoContainerId& operator=(const FIoContainerId& Other) = default;

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

	uint64_t Id;
};