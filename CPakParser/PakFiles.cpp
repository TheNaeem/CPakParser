#include "PakFiles.h"

#include "FileReader.h"
#include "MemoryReader.h"
#include "Hashing.h"
#include "PakReader.h"

//TODO: right now im just directly porting engine code, but once I have everything laid out, I should refactor it to be more practical for my usage of it

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

static void DecodePakEntry(const uint8_t* SourcePtr, FPakEntry& OutEntry, FPakInfo& InInfo)
{
	auto Val = *(uint32_t*)SourcePtr;
	SourcePtr += sizeof(uint32_t);

	uint32_t CompressionBlockSize = 0;
	if ((Val & 0x3f) == 0x3f)
	{
		CompressionBlockSize = *(uint32_t*)SourcePtr;
		SourcePtr += sizeof(uint32_t);
	}
	else
	{
		CompressionBlockSize = ((Val & 0x3f) << 11);
	}

	OutEntry.CompressionMethodIndex = (Val >> 23) & 0x3f;

	bool bIsOffset32BitSafe = (Val & (1 << 31)) != 0;
	if (bIsOffset32BitSafe)
	{
		OutEntry.Offset = *(uint32_t*)SourcePtr;
		SourcePtr += sizeof(uint32_t);
	}
	else
	{
		memcpy(&OutEntry.Offset, SourcePtr, sizeof(int64_t));
		SourcePtr += sizeof(int64_t);
	}

	bool bIsUncompressedSize32BitSafe = (Val & (1 << 30)) != 0;
	if (bIsUncompressedSize32BitSafe)
	{
		OutEntry.UncompressedSize = *(uint32_t*)SourcePtr;
		SourcePtr += sizeof(uint32_t);
	}
	else
	{
		memcpy(&OutEntry.UncompressedSize, SourcePtr, sizeof(int64_t));
		SourcePtr += sizeof(int64_t);
	}

	if (OutEntry.CompressionMethodIndex != 0)
	{
		bool bIsSize32BitSafe = (Val & (1 << 29)) != 0;
		if (bIsSize32BitSafe)
		{
			OutEntry.Size = *(uint32_t*)SourcePtr;
			SourcePtr += sizeof(uint32_t);
		}
		else
		{
			memcpy(&OutEntry.Size, SourcePtr, sizeof(int64_t));
			SourcePtr += sizeof(int64_t);
		}
	}
	else
	{
		OutEntry.Size = OutEntry.UncompressedSize;
	}

	OutEntry.SetEncrypted((Val & (1 << 22)) != 0);

	uint32_t CompressionBlocksCount = (Val >> 6) & 0xffff;
	OutEntry.CompressionBlocks.resize(CompressionBlocksCount);

	OutEntry.CompressionBlockSize = 0;
	if (CompressionBlocksCount > 0)
	{
		OutEntry.CompressionBlockSize = CompressionBlockSize;

		if (CompressionBlocksCount == 1)
		{
			OutEntry.CompressionBlockSize = OutEntry.UncompressedSize;
		}
	}

	OutEntry.SetDeleteRecord(false);

	int64_t BaseOffset = InInfo.HasRelativeCompressedChunkOffsets() ? 0 : OutEntry.Offset;

	if (OutEntry.CompressionBlocks.size() == 1 && !OutEntry.IsEncrypted())
	{
		FPakCompressedBlock& CompressedBlock = OutEntry.CompressionBlocks[0];
		CompressedBlock.CompressedStart = BaseOffset + OutEntry.GetSerializedSize(InInfo.Version);
		CompressedBlock.CompressedEnd = CompressedBlock.CompressedStart + OutEntry.Size;
	}
	else if (OutEntry.CompressionBlocks.size() > 0)
	{
		auto CompressionBlockSizePtr = (uint32_t*)SourcePtr;

		uint64_t CompressedBlockAlignment = OutEntry.IsEncrypted() ? FAESKey::AESBlockSize : 1;

		int64_t CompressedBlockOffset = BaseOffset + OutEntry.GetSerializedSize(InInfo.Version);
		for (int CompressionBlockIndex = 0; CompressionBlockIndex < OutEntry.CompressionBlocks.size(); ++CompressionBlockIndex)
		{
			FPakCompressedBlock& CompressedBlock = OutEntry.CompressionBlocks[CompressionBlockIndex];
			CompressedBlock.CompressedStart = CompressedBlockOffset;
			CompressedBlock.CompressedEnd = CompressedBlockOffset + *CompressionBlockSizePtr++;
			CompressedBlockOffset += Align(CompressedBlock.CompressedEnd - CompressedBlock.CompressedStart, CompressedBlockAlignment);
		}
	}
}

FPakEntry FPakFile::CreateEntry(FPakEntryLocation& Location)
{
	if (Location.IsInvalid())
		return FPakEntry();

	auto EncodedOffset = Location.GetAsOffsetIntoEncoded();

	if (EncodedOffset >= 0)
	{
		FPakEntry Ret;
		DecodePakEntry(EncodedPakEntries.data() + EncodedOffset, Ret, Info);
		return Ret;
	}

	auto ListIdx = Location.GetAsListIndex();
	return Files[ListIdx];
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

int64_t FPakInfo::GetSerializedSize(int32_t InVersion) const
{
	int64_t Size = sizeof(Magic) + sizeof(Version) + sizeof(IndexOffset) + sizeof(IndexSize) + sizeof(FSHAHash) + sizeof(bEncryptedIndex);
	if (InVersion >= PakFile_Version_EncryptionKeyGuid) Size += sizeof(EncryptionKeyGuid);
	if (InVersion >= PakFile_Version_FNameBasedCompressionMethod) Size += CompressionMethodNameLen * MaxNumCompressionMethods;
	if (InVersion >= PakFile_Version_FrozenIndex && InVersion < PakFile_Version_PathHashIndex) Size += sizeof(bool);

	return Size;
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
	Ar.Seek(Ar.Tell() + sizeof(FSHAHash)); // IndexHash

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

	Readers.push_back(SharedReader);
}

FUniqueAr FPakFile::CreateEntryArchive(FFileEntryInfo EntryInfo)
{
	auto PakEntryLoc = *static_cast<FPakEntryLocation*>(&EntryInfo);
	auto Entry = CreateEntry(PakEntryLoc);

	if (Entry.IsEncrypted())
	{
		return std::make_unique<FPakReader<FPakSimpleEncryption>>(shared_from_this(), Entry);
	}

	return std::make_unique<FPakReader<>>(shared_from_this(), Entry);
}

FSharedPakReader FPakFile::GetSharedReader() // TODO: refactor
{
	FArchive* PakReader = nullptr;

	{
		SCOPE_LOCK(CriticalSection);

		if (Readers.size() > 0)
		{
			PakReader = Readers.back();
			Readers.pop_back();
		}
		else
		{
			PakReader = new FFileReader(PakFilePath.string().c_str());
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

	if (MountPoint.back() == '\0')
		MountPoint.pop_back();

	if (MountPoint.length() > 0 && MountPoint[MountPoint.length() - 1] != '/')
	{
		MountPoint += "/";
	}

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

	auto SharedThis = shared_from_this();

	if (!bReadFullDirectoryIndex)
	{
		FGameFileManager::SerializePakIndexes(PathHashIndexReader, MountPoint, SharedThis);
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

		FGameFileManager::SerializePakIndexes(SecondaryIndexReader, MountPoint, SharedThis);
		bHasFullDirectoryIndex = true;
	}

	return true;
}

bool FPakFile::LoadIndex(FArchive& Reader)
{
	if (Info.Version >= FPakInfo::PakFile_Version_PathHashIndex)
	{
		return LoadIndexInternal(Reader);
	}
	
	//TODO: LoadLegacyIndex

	return false;
}

FPakFile::FPakFile(std::filesystem::path FilePath, bool bIsSigned)
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
}

FPakFile::~FPakFile()
{
	for (auto Reader : Readers)
		delete Reader;
}

bool FPakFile::Initialize(bool bLoadIndex)
{
	auto Reader = GetSharedReader();

	if (!Reader) return false;

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

	if (Info.Magic != FPakInfo::PakFile_Magic) return false;

	if (!Info.EncryptionKeyGuid.IsValid() || FEncryptionKeyManager::HasKey(Info.EncryptionKeyGuid))
	{
		if (bLoadIndex) 
			return LoadIndex(Reader.GetArchive());
	}

	return true;
}

FPackageStore::FPackageStore()
{
}

void FPackageStore::Mount(std::shared_ptr<FFilePackageStoreBackend> Backend)
{
	Get().Backends.push_back(Backend);
}
