#pragma once

#include "IoContainerHeader.h"
#include "IoTocResource.h"
#include "GameFileManager.h"
#include "ZenData.h"

class FIoStoreToc : public IDiskFile
{
public:

	FIoStoreToc() // never use this
	{
	}

	FIoStoreToc(FIoStoreTocResource TocRsrc) : FIoStoreToc(std::make_shared<FIoStoreTocResource>(TocRsrc))
	{
	}

	FIoStoreToc(TSharedPtr<FIoStoreTocResource> TocRsrc);

	__forceinline FAESKey& GetEncryptionKey()
	{
		return Key;
	}

	__forceinline TSharedPtr<FIoStoreTocResource> GetResource()
	{
		return Toc;
	}

	std::filesystem::path GetDiskPath() override
	{
		return Toc->TocPath;
	}

	__forceinline void SetKey(FAESKey& InKey)
	{
		Key = InKey;
	}

	__forceinline void SetReader(TSharedPtr<class FIoStoreReader> InReader)
	{
		Reader = InReader;
	}

	__forceinline int32_t GetTocEntryIndex(FIoChunkId& ChunkId)
	{
		return ChunkIdToIndex[ChunkId];
	}

	FIoOffsetAndLength GetOffsetAndLength(FIoChunkId& ChunkId);
	FSharedAr CreateEntryArchive(FFileEntryInfo EntryInfo) override;
	void DoWork(FSharedAr Ar, TSharedPtr<class GContext> Context) override;

private:

	FAESKey Key;
	TSharedPtr<FIoStoreTocResource> Toc;
	TSharedPtr<class FIoStoreReader> Reader;
	phmap::flat_hash_map<FIoChunkId, int32_t> ChunkIdToIndex;
};

class FIoStoreReader : public std::enable_shared_from_this<FIoStoreReader>
{
public:

	FIoStoreReader(const char* ContainerPath, std::atomic_int32_t& PartitionIndex);

	TSharedPtr<FIoStoreToc> Initialize(TSharedPtr<class GContext> Context, bool bSerializeDirectoryIndex = false);

	FIoContainerHeader ReadContainerHeader();

	TUniquePtr<uint8_t[]> Read(FIoChunkId ChunkId);
	TUniquePtr<uint8_t[]> Read(FIoOffsetAndLength& OffsetAndLength);
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

	__forceinline TSharedPtr<FIoStoreToc> GetToc()
	{
		return Toc;
	}

	FZenPackageHeaderData ReadZenPackageHeader(FSharedAr Ar);

private:

	FIoOffsetAndLength FindChunkInternal(FIoChunkId& ChunkId);
	void ParseDirectoryIndex(struct FIoDirectoryIndexResource& DirectoryIndex, FGameFileManager& GameFiles, std::string& Path, uint32_t DirectoryIndexHandle = 0);

	bool bHasPerfectHashMap = false;
	phmap::flat_hash_map<FIoChunkId, FIoOffsetAndLength> TocImperfectHashMapFallback;
	FFileIoStoreContainerFile Container;
	TSharedPtr<FIoStoreToc> Toc;
	FUniqueAr Ar;
	std::mutex Lock;
};