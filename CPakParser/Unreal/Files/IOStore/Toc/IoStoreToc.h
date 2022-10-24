#pragma once

#include "Files/DiskFile.h"
#include "Misc/Encryption/AES.h"
#include "Misc/Hashing/Map.h"
#include "../Misc/IoOffsetAndLength.h"

class FIoStoreToc : public IDiskFile
{
public:

	FIoStoreToc() = default; // never use this

	FIoStoreToc(TSharedPtr<struct FIoStoreTocResource> TocRsrc);

	FAESKey& GetEncryptionKey();
	TSharedPtr<struct FIoStoreTocResource> GetResource();
	std::string GetDiskPath() override;
	void SetKey(FAESKey& InKey);
	void SetReader(TSharedPtr<class FIoStoreReader> InReader);
	int32_t GetTocEntryIndex(struct FIoChunkId& ChunkId);

	FIoOffsetAndLength GetOffsetAndLength(FIoChunkId& ChunkId);
	FSharedAr CreateEntryArchive(FFileEntryInfo EntryInfo) override;
	void DoWork(FSharedAr Ar, TSharedPtr<class GContext> Context) override;

private:

	FAESKey Key;
	TSharedPtr<FIoStoreTocResource> Toc;
	TSharedPtr<class FIoStoreReader> Reader;
	TMap<FIoChunkId, int32_t> ChunkIdToIndex;
};