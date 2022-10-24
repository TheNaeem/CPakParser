#pragma once

#include "PakInfo.h"
#include "Misc/Multithreading/Lock.h"
#include "Misc/Hashing/Map.h"
#include "Serialization/Impl/BorrowedArchive.h"
#include "PakEntryLocation.h"
#include "PakEntry.h"
#include "../DiskFile.h"

// TODO: clean up this class
class FPakFile final : public std::enable_shared_from_this<FPakFile>, public IDiskFile
{
public:

	FPakFile(std::string FilePath, TSharedPtr<class GContext> Context);

	typedef TMap<uint64_t, FPakEntryLocation> FPathHashIndex; 

	enum class ECacheType : uint8_t
	{
		Shared,
		Individual,
	};

	bool Initialize(bool bLoadIndex);

private:

	bool LoadIndex(class FArchive& Reader);
	bool LoadIndexInternal(class FArchive& Reader);
	bool TryDecryptIndex(std::vector<uint8_t>& Data);
	struct FPakEntry CreateEntry(FPakEntryLocation& Location);

	std::string PakFilePath; // TODO: what do i need?
	std::mutex CriticalSection;
	FPakInfo Info;
	std::string MountPoint;
	std::vector<FPakEntry> Files;
	FPathHashIndex PathHashIndex;
	std::vector<uint8_t> EncodedPakEntries;
	uint64_t PathHashSeed;
	int32_t NumEntries;
	int64_t CachedTotalSize;
	bool bIsValid;
	bool bHasPathHashIndex;
	bool bHasFullDirectoryIndex;
	ECacheType CacheType;
	int32_t	CacheIndex;
	bool UnderlyingCacheTrimDisabled;
	bool bIsMounted = false;
	TSharedPtr<class GContext> Context;
	FPakReaderCollection Readers;

public:

	__forceinline FPakInfo& GetInfo()
	{
		return Info;
	}

	__forceinline std::string GetDiskPath() override
	{
		return PakFilePath;
	}

	__forceinline bool IsMounted()
	{
		return bIsMounted;
	}

	__forceinline void SetIsMounted(bool Val)
	{
		bIsMounted = Val;
	}

	__forceinline FBorrowedArchive GetSharedReader()
	{
		return Readers.BorrowReader();
	}

	void DoWork(FSharedAr Ar, TSharedPtr<class GContext> Context) override
	{

	}

	FSharedAr CreateEntryArchive(FFileEntryInfo EntryInfo) override;
};