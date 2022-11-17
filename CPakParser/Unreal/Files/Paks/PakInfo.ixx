export module CPakParser.Paks.PakInfo;

import CPakParser.Misc.FGuid;
import <vector>;
import <string>;

export struct FPakInfo
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

	void Serialize(class FArchive& Ar, int32_t InVersion);
};