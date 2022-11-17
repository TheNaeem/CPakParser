export module CPakParser.Paks.PakEntry;

import <vector>;

export struct FPakCompressedBlock
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

	friend class FArchive& operator<<(FArchive& Ar, FPakCompressedBlock& Block);
};

export struct FPakEntry
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
	void Serialize(class FArchive& Ar, int32_t Version);
};