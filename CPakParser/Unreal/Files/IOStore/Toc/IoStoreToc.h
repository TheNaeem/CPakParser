#pragma once

#include "Misc/Hashing/Map.h"

import CPakParser.IOStore.OffsetAndLength;
import CPakParser.Encryption.AES;
import CPakParser.Files.FileEntry;
import CPakParser.IOStore.ChunkId;

class FIoStoreToc : public IDiskFile
{
public:

	FIoStoreToc() = default; // never use this

	FIoStoreToc(std::string& TocFilePath);
	FIoStoreToc(TSharedPtr<struct FIoStoreTocResource> TocRsrc);

	FAESKey& GetEncryptionKey();
	TSharedPtr<struct FIoStoreTocResource> GetResource();
	std::string GetDiskPath() override;
	void SetKey(FAESKey& InKey);
	void SetReader(TSharedPtr<class FIoStoreReader> InReader);
	int32_t GetTocEntryIndex(FIoChunkId& ChunkId);

	FIoOffsetAndLength GetOffsetAndLength(FIoChunkId& ChunkId);
	FSharedAr CreateEntryArchive(FFileEntryInfo EntryInfo) override;
	void DoWork(FSharedAr Ar, TSharedPtr<class GContext> Context) override;

private:

	FAESKey Key;
	TSharedPtr<struct FIoStoreTocResource> Toc;
	TSharedPtr<class FIoStoreReader> Reader;
	TMap<FIoChunkId, int32_t> ChunkIdToIndex;
};