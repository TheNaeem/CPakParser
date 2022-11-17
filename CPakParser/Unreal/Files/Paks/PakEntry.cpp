import CPakParser.Paks.PakEntry;

import CPakParser.Paks.PakInfo;
import CPakParser.Logging;
import CPakParser.Serialization.FArchive;
import CPakParser.Compression;

FArchive& operator<<(FArchive& Ar, FPakCompressedBlock& Block)
{
	Ar << Block.CompressedStart;
	Ar << Block.CompressedEnd;

	return Ar;
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
		else LogError("Found an unknown compression type in pak file, will need to be supported for legacy files");
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