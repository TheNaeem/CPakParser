#pragma once

#include "Archives.h"
#include "Hashing.h"
#include <mutex>

struct FAESKey;
struct FGuid;
class FChunkCacheWorker;
class IAsyncReadFileHandle;
class FFileIoStore;
class FFilePackageStoreBackend;
struct FIoContainerHeader;

static void MakeDirectoryFromPath(std::string& Path)
{
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
	FSHAHash IndexHash;
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

	int64_t GetSerializedSize(int32_t InVersion = PakFile_Version_Latest) const
	{
		int64_t Size = sizeof(Magic) + sizeof(Version) + sizeof(IndexOffset) + sizeof(IndexSize) + sizeof(IndexHash) + sizeof(bEncryptedIndex);
		if (InVersion >= PakFile_Version_EncryptionKeyGuid) Size += sizeof(EncryptionKeyGuid);
		if (InVersion >= PakFile_Version_FNameBasedCompressionMethod) Size += CompressionMethodNameLen * MaxNumCompressionMethods;
		if (InVersion >= PakFile_Version_FrozenIndex && InVersion < PakFile_Version_PathHashIndex) Size += sizeof(bool);

		return Size;
	}

	std::string GetCompressionMethod(uint8_t Index) const
	{
		return CompressionMethods[Index];
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

	int64_t GetSerializedSize(int32_t Version) const;
	void Serialize(FArchive& Ar, int32_t Version);
};

struct FPakEntryLocation
{
	static const int32_t Invalid = MIN_int32;
	static const int32_t MaxIndex = MAX_int32 - 1;

	FPakEntryLocation() : Index(Invalid)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FPakEntryLocation& Entry) 
	{
		return Ar << Entry.Index;
	}

	friend size_t hash_value(const FPakEntryLocation& in) 
	{
		return (in.Index * 0xdeece66d + 0xb);
	}

	__forceinline friend bool operator==(const FPakEntryLocation& entry1, const FPakEntryLocation& entry2) 
	{
		return entry1.Index == entry2.Index;
	}

private:
	explicit FPakEntryLocation(int32_t InIndex) : Index(InIndex)
	{
	}

	int32_t Index;
};

typedef phmap::flat_hash_map<std::string, FPakEntryLocation> FPakDirectory;

class FPakFile
{
public:
	FPakFile(std::filesystem::path FilePath, bool bIsSigned, bool bLoadIndex);

	typedef phmap::flat_hash_map<uint64_t, FPakEntryLocation> FPathHashIndex; 
	typedef phmap::flat_hash_map<std::string, FPakDirectory> FDirectoryIndex;

	enum class ECacheType : uint8_t
	{
		Shared,
		Individual,
	};

	struct FArchiveAndLastAccessTime
	{
		std::unique_ptr<class FArchive> Archive;
		time_t LastAccessTime;
	};

private:
	void LoadIndex(class FArchive& Reader);
	bool LoadIndexInternal(class FArchive& Reader);
	bool TryDecryptIndex(std::vector<uint8_t>& Data);

	friend class FPakFileManager;

	std::filesystem::path PakFilePath;
	std::vector<FArchiveAndLastAccessTime> Readers;
	int32_t CurrentlyUsedReaders = 0;
	std::mutex CriticalSection;
	FPakInfo Info;
	std::string MountPoint;
	std::vector<FPakEntry> Files;
	FDirectoryIndex FileDirectories;
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

	FPakInfo& GetInfo()
	{
		return Info;
	}

	std::string GetFilename()
	{
		return PakFilePath.filename().string();
	}

	std::filesystem::path& GetPath()
	{
		return PakFilePath;
	}

	bool IsMounted()
	{
		return bIsMounted;
	}

	FSharedPakReader GetSharedReader();
	void ReturnSharedReader(FArchive* SharedReader);
};

class FPakFileManager
{
	std::mutex CriticalSection;
	std::string PaksFolderDir;
	std::vector<std::shared_ptr<FPakFile>> PakFiles;
	bool bSigned;
	bool bIsInitialized = false;
	phmap::flat_hash_set<std::string> ExcludedNonPakExtensions;
	std::string IniFileExtension;
	std::string GameUserSettingsIniFilename;
	std::shared_ptr<class FFileIoStore> IoFileBackend;
	std::shared_ptr<FFilePackageStoreBackend> PackageStoreBackend;
	phmap::flat_hash_map<FGuid, std::filesystem::path> DeferredPaks;

public:
	FPakFileManager();

	bool Initialize(std::string InPaksFolderDir);
	bool Mount(std::filesystem::path InPakFilename, bool bLoadIndex = true);
	void MountAllPakFiles();

	bool RegisterEncryptionKey(FGuid InGuid, FAESKey InKey);
	static FAESKey GetRegisteredPakEncryptionKey(const FGuid& InEncryptionKeyGuid);

	std::vector<std::shared_ptr<FPakFile>> GetMountedPaks()
	{
		return PakFiles;
	}

	void OnPakMounted(std::shared_ptr<FPakFile> Pak)
	{
		SCOPE_LOCK(CriticalSection);

		Pak->bIsMounted = true;

		ReadStatus(ReadErrorCode::Ok, "Sucessfully mounted PAK file: " + Pak->GetFilename());

		PakFiles.push_back(Pak);
	}
};

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

class FEncryptionKeyManager //thread safe encryption key util
{
public:

	static FEncryptionKeyManager& Get()
	{
		static FEncryptionKeyManager Inst;
		return Inst;
	}

	static void AddKey(const FGuid& InGuid, const FAESKey InKey);
	static bool GetKey(const FGuid& InGuid, FAESKey& OutKey);
	static bool const HasKey(const FGuid& InGuid);
	static const phmap::flat_hash_map<FGuid, FAESKey>& GetKeys();

private:
	FEncryptionKeyManager() = default;

	phmap::flat_hash_map<FGuid, FAESKey> Keys;
	std::mutex CriticalSection;
};