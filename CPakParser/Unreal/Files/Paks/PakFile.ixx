module;

#include "Core/Defines.h"

export module CPakParser.Paks.PakFile;

import CPakParser.Paks.PakEntryLocation;
import CPakParser.Paks.PakInfo;
import CPakParser.Serialization.BorrowedArchive;
import CPakParser.Paks.PakEntry;

import <vector>;
import <mutex>;

export class FPakFile final : public std::enable_shared_from_this<FPakFile>, public IDiskFile
{
public:

	FPakFile(std::string FilePath, TSharedPtr<class GContext> Context);

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
	FPakEntry CreateEntry(FPakEntryLocation& Location);

	std::string PakFilePath; // TODO: what do i need?
	std::mutex CriticalSection;
	FPakInfo Info;
	std::string MountPoint;
	std::vector<FPakEntry> Files;
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

	UPackagePtr CreatePackage(class FArchive& Ar, TSharedPtr<class GContext> Context, class FExportState& ExportState) override
	{
		return nullptr; // TODO:
	}

	std::vector<uint8_t> ReadEntry(struct FFileEntryInfo& Entry) override;
};