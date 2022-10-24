#pragma once

#include "IoChunkId.h"
#include "IoOffsetAndLength.h"
#include "Core/Defines.h"
#include "Misc/Hashing/Map.h"
#include "../Container/IoContainer.h"

class FIoStoreToc;

class FIoStoreReader : public std::enable_shared_from_this<FIoStoreReader>
{
public:

	FIoStoreReader(const char* ContainerPath, std::atomic_int32_t& PartitionIndex);

	TSharedPtr<FIoStoreToc> Initialize(TSharedPtr<class GContext> Context, bool bSerializeDirectoryIndex = false);

	FIoContainerHeader ReadContainerHeader();

	TUniquePtr<uint8_t[]> Read(FIoChunkId ChunkId);
	TUniquePtr<uint8_t[]> Read(FIoOffsetAndLength& OffsetAndLength);
	void Read(int32_t InPartitionIndex, int64_t Offset, int64_t Len, uint8_t* OutBuffer);
	bool IsEncrypted() const;

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

	//FZenPackageHeaderData ReadZenPackageHeader(FSharedAr Ar);

private:

	FIoOffsetAndLength FindChunkInternal(FIoChunkId& ChunkId);
	void ParseDirectoryIndex(struct FIoDirectoryIndexResource& DirectoryIndex, class FGameFileManager& GameFiles, std::string& Path, uint32_t DirectoryIndexHandle = 0);

	bool bHasPerfectHashMap = false;
	TMap<FIoChunkId, FIoOffsetAndLength> TocImperfectHashMapFallback;
	FFileIoStoreContainerFile Container;
	TSharedPtr<FIoStoreToc> Toc;
	FUniqueAr Ar;
	std::mutex Lock;
};