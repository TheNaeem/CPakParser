#pragma once

#include "Archives.h"
#include "GameFileManager.h"
#include "IoContainerHeader.h"

static void MakeDirectoryFromPath(std::string& Path)
{
	//Path.erase(Path.find('\0'));

	if (Path.length() > 0 && Path[Path.length() - 1] != '/')
	{
		Path += "/";
	}
}

class FSharedPakReader final
{
	friend class FPakFile;

	FArchive* Archive = nullptr;
	FPakFile* PakFile = nullptr; 

	FSharedPakReader(FArchive* InArchive, FPakFile* InPakFile) : Archive(InArchive), PakFile(InPakFile)
	{
	}

public:
	~FSharedPakReader();

	FSharedPakReader(const FSharedPakReader& Other) = delete;
	FSharedPakReader& operator=(const FSharedPakReader& Other) = delete;

	explicit operator bool() const { return Archive != nullptr; }
	bool operator==(nullptr_t) { return Archive == nullptr; }
	bool operator!=(nullptr_t) { return Archive != nullptr; }
	FArchive* operator->() { return Archive; }

	FArchive& GetArchive() { return *Archive; }
};

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
	uint8_t bEncryptedIndex;
	FGuid EncryptionKeyGuid;
	std::vector<std::string> CompressionMethods;

	FPakInfo()
		: Magic(PakFile_Magic)
		, Version(PakFile_Version_Latest)
		, IndexOffset(-1)
		, IndexSize(0)
		, bEncryptedIndex(0)
	{
		CompressionMethods.push_back("None");
	}

	int64_t GetSerializedSize(int32_t InVersion = PakFile_Version_Latest) const;

	__forceinline std::string GetCompressionMethod(uint8_t Index) const
	{
		return CompressionMethods[Index];
	}

	__forceinline int64_t HasRelativeCompressedChunkOffsets() const
	{
		return Version >= PakFile_Version_RelativeChunkOffsets;
	}

	void Serialize(FArchive& Ar, int32_t InVersion);
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

	friend FArchive& operator<<(FArchive& Ar, FPakCompressedBlock& Block)
	{
		Ar << Block.CompressedStart;
		Ar << Block.CompressedEnd;

		return Ar;
	}
};

struct FPakEntry
{
	FPakEntry();

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

	__forceinline void SetFlag(uint8_t InFlag, bool bValue)
	{
		if (bValue)
		{
			Flags |= InFlag;
		}
		else
		{
			Flags &= ~InFlag;
		}
	}

	__forceinline bool GetFlag(uint8_t InFlag) const
	{
		return (Flags & InFlag) == InFlag;
	}

	__forceinline bool IsEncrypted() const { return GetFlag(Flag_Encrypted); }
	__forceinline void SetEncrypted(bool bEncrypted) { SetFlag(Flag_Encrypted, bEncrypted); }

	__forceinline bool IsDeleteRecord() const { return GetFlag(Flag_Deleted); }
	__forceinline void SetDeleteRecord(bool bDeleteRecord) { SetFlag(Flag_Deleted, bDeleteRecord); }

	int64_t GetSerializedSize(int32_t Version) const;
	void Serialize(FArchive& Ar, int32_t Version);
};

class FPakFile final : public std::enable_shared_from_this<FPakFile>, public IDiskFile
{
public:

	FPakFile(std::filesystem::path FilePath, bool bIsSigned);
	~FPakFile();

	typedef phmap::flat_hash_map<uint64_t, FPakEntryLocation> FPathHashIndex; 

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

	friend class FPakFileManager;

	std::filesystem::path PakFilePath;
	std::vector<FArchive*> Readers;
	int32_t CurrentlyUsedReaders = 0;
	std::mutex CriticalSection;
	FPakInfo Info;
	std::string MountPoint;
	std::vector<FPakEntry> Files;
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

public:

	__forceinline FPakInfo& GetInfo()
	{
		return Info;
	}

	std::string GetFilename()
	{
		return PakFilePath.filename().string();
	}

	std::filesystem::path GetDiskPath() override
	{
		return PakFilePath;
	}

	bool IsMounted()
	{
		return bIsMounted;
	}

	void DoWork(FUniqueAr& Ar) override
	{

	}

	FUniqueAr CreateEntryArchive(FFileEntryInfo EntryInfo) override;
	FSharedPakReader GetSharedReader();
	void ReturnSharedReader(FArchive* SharedReader);
};

// TODO: do i really need this?
class FFilePackageStoreBackend
{
	void Mount(const FIoContainerHeader* ContainerHeader, uint32_t Order);

/*public:
	FFilePackageStoreBackend();
	virtual ~FFilePackageStoreBackend();

	virtual void OnMounted(TSharedRef<const FPackageStoreBackendContext>) override
	{
	}

	virtual void BeginRead() override;
	virtual void EndRead() override;
	virtual EPackageStoreEntryStatus GetPackageStoreEntry(FPackageId PackageId, FPackageStoreEntry& OutPackageStoreEntry) override;
	virtual bool GetPackageRedirectInfo(FPackageId PackageId, FName& OutSourcePackageName, FPackageId& OutRedirectedToPackageId) override;

	
	void Unmount(const FIoContainerHeader* ContainerHeader);

private:
	struct FMountedContainer
	{
		const FIoContainerHeader* ContainerHeader;
		uint32_t Order;
		uint32_t Sequence;
	};

	void Update();

	FRWLock EntriesLock;
	FCriticalSection UpdateLock;
	TArray<FMountedContainer> MountedContainers;
	TAtomic<uint32> NextSequence{ 0 };
	TMap<FPackageId, const FFilePackageStoreEntry*> StoreEntriesMap;
	TMap<FPackageId, TTuple<FName, FPackageId>> RedirectsPackageMap;
	TMap<FPackageId, FName> LocalizedPackages;
	bool bNeedsUpdate = false;*/
};

class FPackageStore : FNoncopyable
{
	FPackageStore();

	std::vector<std::shared_ptr<FFilePackageStoreBackend>> Backends;

public:
	static FPackageStore& Get()
	{
		static FPackageStore Instance;
		return Instance;
	}

	static void Mount(std::shared_ptr<FFilePackageStoreBackend> Backend);
};