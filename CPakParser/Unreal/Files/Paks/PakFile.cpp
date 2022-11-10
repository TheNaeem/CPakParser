#include "PakFile.h"

#include "PakReader.h"
#include "Core/Globals/GlobalContext.h"
#include "Serialization/Impl/MemoryReader.h"
#include "Serialization/Impl/FileReader.h"
#include "Misc/Hashing/ShaHash.h"
#include "Logging.h"

//TODO: right now im just directly porting engine code, but once I have everything laid out, I should refactor it to be more practical for my usage of it

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
		memcpyfst(&OutEntry.Offset, SourcePtr, sizeof(int64_t));
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
		memcpyfst(&OutEntry.UncompressedSize, SourcePtr, sizeof(int64_t));
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
			memcpyfst(&OutEntry.Size, SourcePtr, sizeof(int64_t));
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

int32_t GetPakchunkIndexFromPakFile(std::string InFilename)
{
	const std::string ChunkIdentifier = "pakchunk";
	int32_t Ret = INDEX_NONE;

	int NumIdx = ChunkIdentifier.length();

	if (!InFilename.starts_with(ChunkIdentifier) && !isdigit(InFilename[NumIdx]))
		return Ret;

	return atoi(&ChunkIdentifier.c_str()[NumIdx]);
}

FSharedAr FPakFile::CreateEntryArchive(FFileEntryInfo EntryInfo)
{
	auto PakEntryLoc = *static_cast<FPakEntryLocation*>(&EntryInfo);
	auto Entry = CreateEntry(PakEntryLoc);

	FUniqueAr PakReader = nullptr;

	if (Entry.IsEncrypted())
	{
		PakReader = std::make_unique<FPakReader<FPakSimpleEncryption>>(shared_from_this(), Entry, Context->EncryptionKeyManager);
	}
	else PakReader = std::make_unique<FPakReader<>>(shared_from_this(), Entry, Context->EncryptionKeyManager);

	auto Size = PakReader->TotalSize();
	auto Buffer = malloc(Size);
	PakReader->Serialize(Buffer, Size);

	return std::make_unique<FMemoryReader>(static_cast<uint8_t*>(Buffer), Size, true);
}

bool TryDecryptData(uint8_t* InData, uint32_t InDataSize, FGuid InEncryptionKeyGuid, FEncryptionKeyManager& KeyManager)
{
	FAESKey Key;

	if (KeyManager.GetKey(InEncryptionKeyGuid, Key) && Key.IsValid())
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
		return TryDecryptData(Data.data(), Data.size(), Info.EncryptionKeyGuid, Context->EncryptionKeyManager);
	}

	return true;
}

bool FPakFile::LoadIndexInternal(FArchive& Reader)
{
	bHasPathHashIndex = false;
	bHasFullDirectoryIndex = false;

	if (CachedTotalSize < (Info.IndexOffset + Info.IndexSize))
	{
		LogError("Corrupted index offset in pak file.");
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

	//TODO: throw if mountpoint is empty
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
		LogError("Corrupt pak PrimaryIndex detected!");
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
		Context->FilesManager.SerializePakIndexes(PathHashIndexReader, MountPoint, SharedThis);
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

		Context->FilesManager.SerializePakIndexes(SecondaryIndexReader, MountPoint, SharedThis);
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

FPakFile::FPakFile(std::string FilePath, TSharedPtr<GContext> InContext)
	: PakFilePath(FilePath)
	, PathHashSeed(0)
	, NumEntries(0)
	, CachedTotalSize(0)
	, bIsValid(false)
	, bHasPathHashIndex(false)
	, bHasFullDirectoryIndex(false)
	, CacheType(FPakFile::ECacheType::Shared)
	, CacheIndex(-1)
	, UnderlyingCacheTrimDisabled(false)
	, bIsMounted(false)
	, Context(InContext)
	, Readers(FilePath)
{
}

bool FPakFile::Initialize(bool bLoadIndex)
{
	auto Reader = Readers.BorrowReader();

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

	if (!Info.EncryptionKeyGuid.IsValid() || Context->EncryptionKeyManager.HasKey(Info.EncryptionKeyGuid))
	{
		if (bLoadIndex) 
			return LoadIndex(Reader.GetArchive());
	}

	return true;
}
