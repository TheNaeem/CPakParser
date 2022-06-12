#pragma once

#include "CoreTypes.h"
#include "Hashing.h"
#include <unordered_map>
#include <set>
#include <mutex>

class FChunkCacheWorker;
class IAsyncReadFileHandle;
class FFileIoStore;
class FFilePackageStoreBackend;
struct FIoContainerHeader;

struct FPakInfo
{
	enum
	{
		PakFile_Magic = 0x5A6F12E1,
		MaxChunkDataSize = 64 * 1024,
		CompressionMethodNameLen = 32,
		MaxNumCompressionMethods = 5, 
	};

	enum
	{
		PakFile_Version_Initial = 1,
		PakFile_Version_NoTimestamps = 2,
		PakFile_Version_CompressionEncryption = 3,
		PakFile_Version_IndexEncryption = 4,
		PakFile_Version_RelativeChunkOffsets = 5,
		PakFile_Version_DeleteRecords = 6,
		PakFile_Version_EncryptionKeyGuid = 7,
		PakFile_Version_FNameBasedCompressionMethod = 8,
		PakFile_Version_FrozenIndex = 9,
		PakFile_Version_PathHashIndex = 10,
		PakFile_Version_Fnv64BugFix = 11,


		PakFile_Version_Last,
		PakFile_Version_Invalid,
		PakFile_Version_Latest = PakFile_Version_Last - 1
	};

	uint32_t Magic;
	int32_t Version;
	int64_t IndexOffset;
	int64_t IndexSize;
	FSHAHash IndexHash;
	uint8_t bEncryptedIndex;
	FGuid EncryptionKeyGuid;
	std::vector<FName> CompressionMethods;

	FPakInfo()
		: Magic(PakFile_Magic)
		, Version(PakFile_Version_Latest)
		, IndexOffset(-1)
		, IndexSize(0)
		, bEncryptedIndex(0)
	{
		CompressionMethods.push_back(NAME_None);
	}

	FName GetCompressionMethod(uint8_t Index) const
	{
		return CompressionMethods[Index];
	}
};

struct FPakCompressedBlock
{
	int64_t CompressedStart;
	int64_t CompressedEnd;

	bool operator == (const FPakCompressedBlock& B) const
	{
		return CompressedStart == B.CompressedStart && CompressedEnd == B.CompressedEnd;
	}

	bool operator != (const FPakCompressedBlock& B) const
	{
		return !(*this == B);
	}
};

struct FPakEntry
{
	static const uint8_t Flag_None = 0x00;
	static const uint8_t Flag_Encrypted = 0x01;
	static const uint8_t Flag_Deleted = 0x02;

	int64_t Offset;
	int64_t Size;
	int64_t UncompressedSize;
	uint8_t Hash[20];
	std::vector<FPakCompressedBlock> CompressionBlocks;
	uint32_t CompressionBlockSize;
	uint32_t CompressionMethodIndex;
	uint8_t Flags;
	mutable bool Verified;
};

struct FPakEntryLocation
{
	static const int32_t Invalid = MIN_int32;
	static const int32_t MaxIndex = MAX_int32 - 1;

	FPakEntryLocation() : Index(Invalid)
	{
	}

private:
	explicit FPakEntryLocation(int32_t InIndex) : Index(InIndex)
	{
	}

	int32_t Index;
};

typedef std::unordered_map<std::string, FPakEntryLocation> FPakDirectory;

class FPakFile : FNoncopyable
{
public:
	typedef std::unordered_map<uint64_t, FPakEntryLocation> FPathHashIndex; 
	typedef std::unordered_map<std::string, FPakDirectory> FDirectoryIndex;

	enum class ECacheType : uint8_t
	{
		Shared,
		Individual,
	};

	struct FArchiveAndLastAccessTime
	{
		std::unique_ptr<FArchive> Archive;
		double LastAccessTime;
	};

private:
	friend class FPakPlatformFile;

	std::string PakFilename;
	FName PakFilenameName;
	std::unique_ptr<class FChunkCacheWorker> Decryptor;
	std::vector<FArchiveAndLastAccessTime> Readers;
	int32_t CurrentlyUsedReaders = 0;
	std::mutex CriticalSection;
	FPakInfo Info;
	std::string MountPoint;
	std::vector<FPakEntry> Files;
	FDirectoryIndex DirectoryIndex;
	FPathHashIndex PathHashIndex;
	std::vector<uint8_t> EncodedPakEntries;
	uint64_t PathHashSeed;
	int32_t NumEntries;
	time_t Timestamp;
	int64_t CachedTotalSize;
	bool bSigned;
	bool bIsValid;
	bool bHasPathHashIndex;
	bool bHasFullDirectoryIndex;
	int32_t PakchunkIndex;
	class IMappedFileHandle* MappedFileHandle;
	std::mutex MappedFileHandleCriticalSection;
	ECacheType	CacheType;
	int32_t	CacheIndex;
	bool UnderlyingCacheTrimDisabled;
	bool bIsMounted;
	std::unique_ptr<FIoContainerHeader> IoContainerHeader;
};

class FPakPlatformFile
{
	struct FPakListEntry
	{
		FPakListEntry()
			: ReadOrder(0)
			, PakFile(nullptr)
		{}

		unsigned int ReadOrder;
		std::shared_ptr<FPakFile> PakFile;

		__forceinline bool operator <(const FPakListEntry& RHS) const
		{
			return ReadOrder > RHS.ReadOrder;
		}
	};

	struct FPakListDeferredEntry
	{
		const char* Filename;
		const char* Path;
		unsigned int ReadOrder;
		FGuid EncryptionKeyGuid;
		unsigned int PakchunkIndex;
	};

	FPakPlatformFile* LowerLevel;
	std::vector<FPakListEntry> PakFiles;
	std::vector<FPakListDeferredEntry> PendingEncryptedPakFiles;
	bool bSigned;
	std::set<std::string> ExcludedNonPakExtensions;
	std::string IniFileExtension;
	std::string GameUserSettingsIniFilename;
	std::shared_ptr<FFileIoStore> IoDispatcherFileBackend;
	std::shared_ptr<FFilePackageStoreBackend> PackageStoreBackend;
};