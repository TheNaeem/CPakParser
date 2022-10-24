#pragma once

#include "Core/Defines.h"
#include "Misc/Encryption/AES.h"
#include "IoContainerHeader.h"

struct FFileIoStoreContainerFilePartition
{
	FFileIoStoreContainerFilePartition() = default;
	FFileIoStoreContainerFilePartition(FFileIoStoreContainerFilePartition&&) = default;
	FFileIoStoreContainerFilePartition(const FFileIoStoreContainerFilePartition&) = delete;

	FFileIoStoreContainerFilePartition& operator=(FFileIoStoreContainerFilePartition&&) = default;
	FFileIoStoreContainerFilePartition& operator=(const FFileIoStoreContainerFilePartition&) = delete;

	bool OpenContainer(const char* ContainerFilePath);

	FUniqueAr Ar;
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

	FIoContainerHeader Header;
	TSharedPtr<struct FIoStoreTocResource> TocResource;
	std::string FilePath;
	FAESKey EncryptionKey;
	std::vector<FFileIoStoreContainerFilePartition> Partitions;
};