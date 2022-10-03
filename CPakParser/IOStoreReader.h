#pragma once

#include "IoContainerHeader.h"
#include "IoTocResource.h"
#include "GameFileManager.h"

class FFileIoStore final
{
public:

	static std::atomic_uint32_t GlobalPartitionIndex;
	static std::atomic_uint32_t GlobalContainerInstanceId;

	FIoContainerHeader Mount(std::string InTocPath, FGuid EncryptionKeyGuid, FAESKey EncryptionKey);
	void Initialize();

private:
	uint64_t ReadBufferSize = 0;
};

class FIoStoreToc : public IDiskFile
{
public:

	FIoStoreToc() // never use this
	{
	}

	FIoStoreToc(FIoStoreTocResource TocRsrc) : FIoStoreToc(std::make_shared<FIoStoreTocResource>(TocRsrc))
	{
	}

	FIoStoreToc(std::shared_ptr<FIoStoreTocResource> TocRsrc);

	__forceinline FAESKey& GetEncryptionKey()
	{
		return Key;
	}

	__forceinline std::shared_ptr<FIoStoreTocResource> GetResource()
	{
		return Toc;
	}

	std::filesystem::path GetDiskPath() override
	{
		return Toc->TocPath;
	}

	__forceinline void SetReader(std::shared_ptr<class FIoStoreReader> InReader)
	{
		Reader = InReader;
	}

	FUniqueAr CreateEntryArchive(FFileEntryInfo EntryInfo) override;
	void DoWork(FUniqueAr& Ar) override;

private:

	FAESKey Key;
	std::shared_ptr<FIoStoreTocResource> Toc;
	std::shared_ptr<class FIoStoreReader> Reader;
};

class FIoStoreReader : public std::enable_shared_from_this<FIoStoreReader>
{
public:

	FIoStoreReader(const char* ContainerPath);

	void Initialize(bool bSerializeDirectoryIndex = false);

	FIoContainerHeader ReadContainerHeader();
	FIoStoreTocChunkInfo CreateTocChunkInfo(uint32_t TocEntryIndex);

	void Read(int32_t InPartitionIndex, int64_t Offset, int64_t Len, uint8_t* OutBuffer);

	__forceinline bool IsEncrypted() const 
	{ 
		return Container.TocResource->Header.IsEncrypted();
	}

	__forceinline void SetEncryptionKey(const FAESKey& Key) 
	{ 
		Container.EncryptionKey = Key;
	}

	__forceinline FFileIoStoreContainerFile& GetContainer()
	{
		return Container;
	}

private:

	FIoOffsetAndLength FindChunkInternal(FIoChunkId& ChunkId);
	void ParseDirectoryIndex(struct FIoDirectoryIndexResource& DirectoryIndex, std::string& Path, uint32_t DirectoryIndexHandle = 0);

	bool bHasPerfectHashMap = false;
	phmap::flat_hash_map<FIoChunkId, FIoOffsetAndLength> TocImperfectHashMapFallback;
	FFileIoStoreContainerFile Container;
	std::shared_ptr<FIoStoreToc> Toc;
	FUniqueAr Ar;
	std::mutex Lock;
};