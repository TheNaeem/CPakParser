#pragma once

#include "Core/Defines.h"
#include "Misc/Hashing/Map.h"
#include "../Container/IoContainer.h"

import CPakParser.IOStore.OffsetAndLength;
import CPakParser.IOStore.ChunkId;

class FIoStoreToc;

class FIoStoreReader : public std::enable_shared_from_this<FIoStoreReader>
{
public:

	FIoStoreReader(TSharedPtr<FIoStoreToc> InToc, std::atomic_int32_t& PartitionIndex);

	void Initialize(TSharedPtr<class GContext> Context, bool bSerializeDirectoryIndex = false);

	void ReadContainerHeader();

	TUniquePtr<uint8_t[]> Read(FIoChunkId ChunkId);
	void Read(uint64_t Offset, uint64_t Length, uint8_t* OutBuffer);
	void Read(int32_t InPartitionIndex, int64_t Offset, int64_t Len, uint8_t* OutBuffer);
	bool IsEncrypted() const;
	
	inline TUniquePtr<uint8_t[]> Read(uint64_t Offset, uint64_t Length)
	{
		auto Ret = std::make_unique<uint8_t[]>(Length);
		Read(Offset, Length, Ret.get());

		return Ret;
	}

	inline TUniquePtr<uint8_t[]> Read(FIoOffsetAndLength& OffsetAndLength)
	{
		return Read(OffsetAndLength.GetOffset(), OffsetAndLength.GetLength());
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
		return Toc.lock();
	}

	//FZenPackageHeaderData ReadZenPackageHeader(FSharedAr Ar);

private:

	FIoOffsetAndLength FindChunkInternal(FIoChunkId& ChunkId);
	void ParseDirectoryIndex(struct FIoDirectoryIndexResource& DirectoryIndex, class FGameFileManager& GameFiles, std::string& Path, uint32_t DirectoryIndexHandle = 0);

	bool bHasPerfectHashMap = false;
	TMap<FIoChunkId, FIoOffsetAndLength> TocImperfectHashMapFallback;
	FFileIoStoreContainerFile Container;
	TWeakPtr<FIoStoreToc> Toc;
	FUniqueAr Ar;
	std::mutex Lock;
};