#include "PakFiles.h"

#include "IOStoreReader.h"
#include "ArchiveFileReaderGeneric.h"
#include "MemoryReader.h"

#include <future>

//TODO: right now im just directly porting engine code, but once I have everything laid out, I should refactor it to be more practical for my usage of it

void FEncryptionKeyManager::AddKey(const FGuid& InGuid, const FAESKey InKey)
{
	auto& KeyCache = Get();

	SCOPE_LOCK(KeyCache.CriticalSection);

	if (!KeyCache.Keys.contains(InGuid))
	{
		KeyCache.Keys.insert_or_assign(InGuid, InKey);
	}
}

bool FEncryptionKeyManager::GetKey(const FGuid& InGuid, FAESKey& OutKey)
{
	auto& KeyCache = Get();

	SCOPE_LOCK(KeyCache.CriticalSection);

	if (!KeyCache.Keys.contains(InGuid)) return false;

	OutKey = KeyCache.Keys[InGuid];
	return true;
}

bool const FEncryptionKeyManager::HasKey(const FGuid& InGuid)
{
	return Get().Keys.contains(InGuid);
}

const phmap::flat_hash_map<FGuid, FAESKey>& FEncryptionKeyManager::GetKeys()
{
	return Get().Keys;
}

FPakEntry::FPakEntry() :
	Offset(-1),
	Size(0),
	UncompressedSize(0),
	CompressionBlockSize(0),
	CompressionMethodIndex(0),
	Flags(Flag_None),
	Verified(false)
{
	memset(Hash, 0, sizeof(Hash));
	CompressionBlocks.clear();
}

int64_t FPakEntry::GetSerializedSize(int32_t Version) const
{
	int64_t SerializedSize = sizeof(Offset) + sizeof(Size) + sizeof(UncompressedSize) + sizeof(Hash);

	if (Version >= FPakInfo::PakFile_Version_FNameBasedCompressionMethod)
	{
		SerializedSize += sizeof(CompressionMethodIndex);
	}
	else
	{
		SerializedSize += sizeof(int32_t);
	}

	if (Version >= FPakInfo::PakFile_Version_CompressionEncryption)
	{
		SerializedSize += sizeof(Flags) + sizeof(CompressionBlockSize);
		if (CompressionMethodIndex != 0)
		{
			SerializedSize += sizeof(FPakCompressedBlock) * CompressionBlocks.size() + sizeof(int32_t);
		}
	}
	if (Version < FPakInfo::PakFile_Version_NoTimestamps)
	{
		SerializedSize += sizeof(int64_t);
	}
	return SerializedSize;
}

void FPakEntry::Serialize(FArchive& Ar, int32_t Version)
{
	struct OffsetSizeProps
	{
		int64_t Offset;
		int64_t Size;
		int64_t UncompressedSize;
	};

	Ar.BulkSerialize<OffsetSizeProps>(&Offset);

	if (Version < FPakInfo::PakFile_Version_FNameBasedCompressionMethod)
	{
		int32_t LegacyCompressionMethod;
		Ar << LegacyCompressionMethod;

		if (LegacyCompressionMethod == COMPRESS_None) CompressionMethodIndex = 0;
		else if (LegacyCompressionMethod & COMPRESS_ZLIB) CompressionMethodIndex = 1;
		else if (LegacyCompressionMethod & COMPRESS_GZIP) CompressionMethodIndex = 2;
		else if (LegacyCompressionMethod & COMPRESS_Custom) CompressionMethodIndex = 3;
		else ReadStatus(ReadErrorCode::CompressionError, "Found an unknown compression type in pak file, will need to be supported for legacy files");
	}
	else
	{
		Ar << CompressionMethodIndex;
	}

	if (Version <= FPakInfo::PakFile_Version_Initial)
	{
		int64_t Timestamp;
		Ar << Timestamp;
	}

	Ar.Serialize(Hash, sizeof(Hash));

	if (Version >= FPakInfo::PakFile_Version_CompressionEncryption)
	{
		if (CompressionMethodIndex != 0)
		{
			Ar << CompressionBlocks;
		}

		Ar << Flags;
		Ar << CompressionBlockSize;
	}
}

void FPakInfo::Serialize(FArchive& Ar, int32_t InVersion)
{
	if (Ar.TotalSize() < (Ar.Tell() + GetSerializedSize(InVersion)))
	{
		Magic = 0;
		return;
	}

	if (InVersion >= PakFile_Version_EncryptionKeyGuid)
	{
		Ar << EncryptionKeyGuid;
	}

	Ar << bEncryptedIndex;
	Ar << Magic;

	if (Magic != PakFile_Magic)
		return;

	Ar << Version;
	Ar << IndexOffset;
	Ar << IndexSize;
	Ar << IndexHash;

	if (Version < PakFile_Version_IndexEncryption)
	{
		bEncryptedIndex = false;
	}

	if (Version < PakFile_Version_EncryptionKeyGuid)
	{
		EncryptionKeyGuid.Invalidate();
	}

	if (Version >= PakFile_Version_FrozenIndex && Version < PakFile_Version_PathHashIndex)
	{
		bool bIndexIsFrozen = false;
		Ar << bIndexIsFrozen;

		if (bIndexIsFrozen)
			ReadStatus(ReadErrorCode::ReadError, "Pak frozen with unsupported version");
	}

	if (Version < PakFile_Version_FNameBasedCompressionMethod)
	{
		CompressionMethods.push_back("Zlib");
		CompressionMethods.push_back("Gzip");
		CompressionMethods.push_back("Oodle");

		return;
	}

	int BufSize = CompressionMethodNameLen * MaxNumCompressionMethods;
	auto Methods = std::make_unique<char[]>(BufSize);

	Ar.Serialize(Methods.get(), BufSize);

	for (size_t i = 0; i < MaxNumCompressionMethods; i++)
	{
		const char* MethodString = &Methods[i * CompressionMethodNameLen];

		if (MethodString[0] != 0)
		{
			CompressionMethods.push_back(MethodString);
		}
	}
}

FSharedPakReader::~FSharedPakReader()
{
	if (Archive)
	{
		PakFile->ReturnSharedReader(Archive);
		Archive = nullptr;
	}
}

FPakFileManager::FPakFileManager() : bSigned(false)
{
}

int32_t GetPakchunkIndexFromPakFile(std::string InFilename)
{
	const std::string ChunkIdentifier = "pakchunk";
	int32_t Ret = INDEX_NONE;

	int NumIdx = ChunkIdentifier.length();

	if (!InFilename.starts_with(ChunkIdentifier) && !isdigit(InFilename[NumIdx]))
		return Ret;

	return atoi(&ChunkIdentifier.c_str()[NumIdx]);
}

void FPakFile::ReturnSharedReader(FArchive* SharedReader)
{
	SCOPE_LOCK(CriticalSection);

	--CurrentlyUsedReaders;

	Readers.push_back(
		FArchiveAndLastAccessTime
		{
			std::make_unique<FArchive>(*SharedReader),
			time(NULL)
		});
}

FSharedPakReader FPakFile::GetSharedReader()
{
	FArchive* PakReader = nullptr;

	{
		SCOPE_LOCK(CriticalSection);

		if (Readers.size() > 0)
		{
			PakReader = Readers.back().Archive.release();
			Readers.pop_back();
		}
		else
		{
			PakReader = std::make_unique<FArchiveFileReaderGeneric>(PakFilePath.string().c_str()).release();
		}

		++CurrentlyUsedReaders;
	}

	return FSharedPakReader(PakReader, this);
}

bool TryDecryptData(uint8_t* InData, uint32_t InDataSize, FGuid InEncryptionKeyGuid)
{
	FAESKey Key;

	if (FEncryptionKeyManager::GetKey(InEncryptionKeyGuid, Key) && Key.IsValid())
	{
		Key.DecryptData(InData, InDataSize);
		return true;
	}

	return false;
}

bool FPakFile::TryDecryptIndex(std::vector<uint8_t>& Data)
{
	if (Info.bEncryptedIndex)
	{
		return TryDecryptData(Data.data(), Data.size(), Info.EncryptionKeyGuid);
	}

	return true;
}

bool FPakFile::LoadIndexInternal(FArchive& Reader)
{
	bHasPathHashIndex = false;
	bHasFullDirectoryIndex = false;

	if (CachedTotalSize < (Info.IndexOffset + Info.IndexSize))
	{
		ReadStatus(ReadErrorCode::CorruptFile, "Corrupted index offset in pak file.");
		return false;
	}

	Reader.Seek(Info.IndexOffset);

	auto PrimaryIndexData = std::vector<uint8_t>();
	PrimaryIndexData.resize(Info.IndexSize);

	Reader.Serialize(PrimaryIndexData.data(), Info.IndexSize);

	if (!TryDecryptIndex(PrimaryIndexData))
	{
		return false;
	}

	FMemoryReader PrimaryIndexReader(PrimaryIndexData);

	NumEntries = 0;
	PrimaryIndexReader << MountPoint;
	MakeDirectoryFromPath(MountPoint);
	PrimaryIndexReader << NumEntries;
	PrimaryIndexReader << PathHashSeed;

	int64_t PathHashIdxOffset = INDEX_NONE;
	int64_t PathHashIdxSize = 0;
	FSHAHash PathHashIdxHash;

	bool bReaderHasPathHashIndex = false;
	PrimaryIndexReader << bReaderHasPathHashIndex;

	if (bReaderHasPathHashIndex)
	{
		PrimaryIndexReader << PathHashIdxOffset;
		PrimaryIndexReader << PathHashIdxSize;
		PrimaryIndexReader << PathHashIdxHash;

		bReaderHasPathHashIndex = bReaderHasPathHashIndex && PathHashIdxOffset != INDEX_NONE;
	}

	int64_t FullDirectoryIdxOffset = INDEX_NONE;
	int64_t FullDirectoryIdxSize = 0;
	FSHAHash FullDirectoryIdxHash;

	bool bReaderHasFullDirectoryIndex = false;
	PrimaryIndexReader << bReaderHasFullDirectoryIndex;

	if (bReaderHasFullDirectoryIndex)
	{
		PrimaryIndexReader << FullDirectoryIdxOffset;
		PrimaryIndexReader << FullDirectoryIdxSize;
		PrimaryIndexReader << FullDirectoryIdxHash;

		bReaderHasFullDirectoryIndex = bReaderHasFullDirectoryIndex && FullDirectoryIdxOffset != INDEX_NONE;
	}

	PrimaryIndexReader << EncodedPakEntries;

	int32_t FilesNum = 0;
	PrimaryIndexReader << FilesNum;

	if (0 > FilesNum) return false;

	Files.resize(FilesNum);

	if (FilesNum)
	{
		for (auto Entry : Files)
		{
			Entry.Serialize(PrimaryIndexReader, Info.Version);
		}
	}

	bool bWillUseFullDirectoryIndex;
	bool bWillUsePathHashIndex;
	bool bReadFullDirectoryIndex;

	if (bReaderHasPathHashIndex && bReaderHasFullDirectoryIndex)
	{
		bWillUseFullDirectoryIndex = true;
		bWillUsePathHashIndex = !bWillUseFullDirectoryIndex;
		bReadFullDirectoryIndex = bReaderHasFullDirectoryIndex && bWillUseFullDirectoryIndex;
	}
	else if (bReaderHasPathHashIndex)
	{
		bWillUsePathHashIndex = true;
		bWillUseFullDirectoryIndex = false;
		bReadFullDirectoryIndex = false;
	}
	else if (bReaderHasFullDirectoryIndex)
	{
		bWillUsePathHashIndex = false;
		bWillUseFullDirectoryIndex = true;
		bReadFullDirectoryIndex = true;
	}
	else
	{
		ReadStatus(ReadErrorCode::CorruptFile, "Corrupt pak PrimaryIndex detected!");
		return false;
	}

	std::vector<uint8_t> PathHashIndexData;
	FMemoryReader PathHashIndexReader(PathHashIndexData);

	if (bWillUsePathHashIndex)
	{
		Reader.Seek(PathHashIdxOffset);

		PathHashIndexData.resize(PathHashIdxSize);
		Reader.Serialize(PathHashIndexData.data(), PathHashIdxSize);

		if (!TryDecryptIndex(PathHashIndexData)) return false;

		PathHashIndexReader << PathHashIndex;
		bHasPathHashIndex = true;
	}

	if (!bReadFullDirectoryIndex)
	{
		PathHashIndexReader << FileDirectories;
		bHasFullDirectoryIndex = false;
	}
	else
	{
		std::vector<uint8_t> FullDirectoryIdxData;
		FullDirectoryIdxData.resize(FullDirectoryIdxSize);

		Reader.Seek(FullDirectoryIdxOffset);
		Reader.Serialize(FullDirectoryIdxData.data(), FullDirectoryIdxSize);

		if (!TryDecryptIndex(FullDirectoryIdxData))
		{
			return false;
		}

		FMemoryReader SecondaryIndexReader(FullDirectoryIdxData);

		SecondaryIndexReader << FileDirectories;
		bHasFullDirectoryIndex = true;
	}

	return true;
}

void FPakFile::LoadIndex(FArchive& Reader)
{
	if (Info.Version >= FPakInfo::PakFile_Version_PathHashIndex)
	{
		LoadIndexInternal(Reader);
	}
	else
	{
		//TODO: LoadLegacyIndex
	}
}

FPakFile::FPakFile(std::filesystem::path FilePath, bool bIsSigned, bool bLoadIndex)
	: PakFilePath(FilePath)
	, PathHashSeed(0)
	, NumEntries(0)
	, CachedTotalSize(0)
	, bSigned(bIsSigned)
	, bIsValid(false)
	, bHasPathHashIndex(false)
	, bHasFullDirectoryIndex(false)
	, PakchunkIndex(GetPakchunkIndexFromPakFile(FilePath.filename().string()))
	, MappedFileHandle(nullptr)
	, CacheType(FPakFile::ECacheType::Shared)
	, CacheIndex(-1)
	, UnderlyingCacheTrimDisabled(false)
	, bIsMounted(false)
{
	auto Reader = GetSharedReader();

	if (!Reader) return;

	CachedTotalSize = Reader->TotalSize();

	int32_t CompatibleVersion = FPakInfo::PakFile_Version_Latest + 1;

	do
	{
		CompatibleVersion--;

		auto InfoPos = CachedTotalSize - Info.GetSerializedSize(CompatibleVersion);

		if (!InfoPos) continue;

		Reader->Seek(InfoPos);

		Info.Serialize(Reader.GetArchive(), CompatibleVersion);

		if (Info.Magic == FPakInfo::PakFile_Magic)
		{
			break;
		}
	} while (CompatibleVersion >= FPakInfo::PakFile_Version_Initial);

	if (Info.Magic != FPakInfo::PakFile_Magic) return;

	if (!Info.EncryptionKeyGuid.IsValid() || FEncryptionKeyManager::HasKey(Info.EncryptionKeyGuid))
	{
		if (bLoadIndex) LoadIndex(Reader.GetArchive());
	}
}

FPackageStore::FPackageStore()
{
}

void FPackageStore::Mount(std::shared_ptr<FFilePackageStoreBackend> Backend)
{
	Get().Backends.push_back(Backend);
}

bool FPakFileManager::RegisterEncryptionKey(FGuid InGuid, FAESKey InKey)
{
	if (FEncryptionKeyManager::HasKey(InGuid))
	{
		ReadStatus(ReadErrorCode::InvalidEncryptionKey, "Attempting to register a key with a GUID that has already been registered.");
		return false;
	}

	FEncryptionKeyManager::AddKey(InGuid, InKey);

	if (!bIsInitialized) return true; // if the pak manager hasn't been initialized yet, the key will get cached and used when the dataminer gets initialized

	if (!DeferredPaks.contains(InGuid)) return false;

	auto DynamicPakPath = DeferredPaks[InGuid];

	if (Mount(DynamicPakPath))
	{
		SCOPE_LOCK(CriticalSection);
		DeferredPaks.erase(InGuid);
	}
	else return false;

	return true;
}

FAESKey FPakFileManager::GetRegisteredPakEncryptionKey(const FGuid& InEncryptionKeyGuid)
{
	FAESKey Ret;

	if (!FEncryptionKeyManager::GetKey(InEncryptionKeyGuid, Ret))
	{
		//
	}

	return Ret;
}

bool FPakFileManager::Initialize(std::string InPaksFolderDir)
{
	if (bIsInitialized) return true;

	PaksFolderDir = InPaksFolderDir;

	ExcludedNonPakExtensions.insert("uasset");
	ExcludedNonPakExtensions.insert("umap");
	ExcludedNonPakExtensions.insert("ubulk");
	ExcludedNonPakExtensions.insert("uexp");
	ExcludedNonPakExtensions.insert("uptnl");
	ExcludedNonPakExtensions.insert("ushaderbytecode");

	auto GlobalUTocPath = std::filesystem::path(PaksFolderDir) /= "global.utoc";

	if (std::filesystem::exists(GlobalUTocPath))
	{
		ReadStatus(ReadErrorCode::Ok, "Mounting Global");

		IoFileBackend = std::make_shared<FFileIoStore>();
		PackageStoreBackend = std::make_shared<FFilePackageStoreBackend>();
		FPackageStore::Mount(PackageStoreBackend);

		IoFileBackend->Mount(GlobalUTocPath.string(), FGuid(), FAESKey());

		ReadStatus(ReadErrorCode::Ok, "Successfully mounted Global TOC");
	}

	this->MountAllPakFiles();

	return bIsInitialized = true;
}

bool FPakFileManager::Mount(std::filesystem::path InPakFilePath, bool bLoadIndex) //TODO: attempt to mount a toc even 
{
	auto PakFilename = InPakFilePath.filename().string();

	ReadStatus(ReadErrorCode::Ok, "Mounting PAK file: " + PakFilename);

	auto Pak = std::make_shared<FPakFile>(InPakFilePath, bSigned, bLoadIndex);

	auto PakGuid = Pak->GetInfo().EncryptionKeyGuid;

	if (FEncryptionKeyManager::HasKey(PakGuid) || !PakGuid.IsValid())
	{
		FAESKey Key;
		FEncryptionKeyManager::GetKey(PakGuid, Key);

		auto TocPath = InPakFilePath.replace_extension(".utoc");

		if (std::filesystem::exists(TocPath))
		{
			auto Container = IoFileBackend->Mount(TocPath.string(), PakGuid, Key);

			if (Container.IsValid())
			{
				Pak->IoContainerHeader = std::make_unique<FIoContainerHeader>(Container);
			}
		}
	}
	else
	{
		SCOPE_LOCK(CriticalSection);

		if (!DeferredPaks.contains(PakGuid))
		{
			DeferredPaks.insert_or_assign(PakGuid, InPakFilePath);
		}

		ReadStatus(ReadErrorCode::Cancelled, "Encryption key could not be found for " + PakFilename + " so it will be deferred until it's registered");

		return false;
	}

	OnPakMounted(Pak);

	return true;
}

void FPakFileManager::MountAllPakFiles() 
{
	std::vector<std::filesystem::path> PakFiles;
	for (auto& File : std::filesystem::directory_iterator(PaksFolderDir))
	{
		if (File.path().extension() == ".pak")
			PakFiles.push_back(File.path());
	}

	auto MountedPaks = GetMountedPaks();

	phmap::flat_hash_set<std::filesystem::path> MountedPakNames;

	for (auto Pak : MountedPaks)
	{
		MountedPakNames.insert(Pak->GetPath());
	}

#define ASYNC_MOUNT 1

	{
#if ASYNC_MOUNT
		std::vector<std::future<void>> Futures;
#endif

		for (auto PakPath : PakFiles)
		{
			if (MountedPakNames.contains(PakPath))
				continue;

#if ASYNC_MOUNT
			Futures.push_back(std::async(std::launch::async, [=]
				{
#endif
					if (!Mount(PakPath)) 
					{
						ReadStatus(ReadErrorCode::Cancelled, "Could not mount PAK file: " + PakPath.filename().string());
					}
#if ASYNC_MOUNT
				}));
#endif
		}
	}

	ReadStatus(ReadErrorCode::Ok, "Mounted all available PAK files");
}
