#pragma once

#include "IoContainer.h"
#include "IoChunkId.h"
#include "Hashing.h"

struct FIoStoreTocCompressedBlockEntry
{
	static constexpr uint32_t OffsetBits = 40;
	static constexpr uint64_t OffsetMask = (1ull << OffsetBits) - 1ull;
	static constexpr uint32_t SizeBits = 24;
	static constexpr uint32_t SizeMask = (1 << SizeBits) - 1;
	static constexpr uint32_t SizeShift = 8;

	inline uint64_t GetOffset() const
	{
		const uint64_t* Offset = reinterpret_cast<const uint64_t*>(Data);
		return *Offset & OffsetMask;
	}

	inline void SetOffset(uint64_t InOffset)
	{
		uint64_t* Offset = reinterpret_cast<uint64_t*>(Data);
		*Offset = InOffset & OffsetMask;
	}

	inline uint32_t GetCompressedSize() const
	{
		const uint32_t* Size = reinterpret_cast<const uint32_t*>(Data) + 1;
		return (*Size >> SizeShift) & SizeMask;
	}

	inline void SetCompressedSize(uint32_t InSize)
	{
		uint32_t* Size = reinterpret_cast<uint32_t*>(Data) + 1;
		*Size |= (uint32_t(InSize) << SizeShift);
	}

	inline uint32_t GetUncompressedSize() const
	{
		const uint32_t* UncompressedSize = reinterpret_cast<const uint32_t*>(Data) + 2;
		return *UncompressedSize & SizeMask;
	}

	inline void SetUncompressedSize(uint32_t InSize)
	{
		uint32_t* UncompressedSize = reinterpret_cast<uint32_t*>(Data) + 2;
		*UncompressedSize = InSize & SizeMask;
	}

	__forceinline uint8_t GetCompressionMethodIndex() const
	{
		const uint32_t* Index = reinterpret_cast<const uint32_t*>(Data) + 2;
		return static_cast<uint8_t>(*Index >> SizeBits);
	}

	inline void SetCompressionMethodIndex(uint8_t InIndex)
	{
		uint32_t* Index = reinterpret_cast<uint32_t*>(Data) + 2;
		*Index |= uint32_t(InIndex) << SizeBits;
	}

private:
	/* 5 bytes offset, 3 bytes for size / uncompressed size and 1 byte for compression method. */
	uint8_t Data[5 + 3 + 3 + 1];
};

struct FIoStoreTocHeader
{
	static constexpr char TocMagicImg[] = "-==--==--==--==-";

	uint8_t TocMagic[16];
	uint8_t	Version;
	uint8_t	Reserved0 = 0;
	uint16_t Reserved1 = 0;
	uint32_t TocHeaderSize;
	uint32_t TocEntryCount;
	uint32_t TocCompressedBlockEntryCount;
	uint32_t TocCompressedBlockEntrySize;
	uint32_t CompressionMethodNameCount;
	uint32_t CompressionMethodNameLength;
	uint32_t CompressionBlockSize;
	uint32_t DirectoryIndexSize;
	uint32_t PartitionCount = 0;
	FIoContainerId ContainerId;
	FGuid EncryptionKeyGuid;
	EIoContainerFlags ContainerFlags;
	uint8_t	Reserved3 = 0;
	uint16_t Reserved4 = 0;
	uint32_t TocChunkPerfectHashSeedsCount = 0;
	uint64_t PartitionSize = 0;
	uint32_t TocChunksWithoutPerfectHashCount = 0;
	uint32_t Reserved7 = 0;
	uint64_t Reserved8[5] = { 0 };

	__forceinline friend FArchive& operator<<(FArchive& Ar, FIoStoreTocHeader& EntryInfo)
	{
		Ar.Serialize(&EntryInfo, sizeof(FIoStoreTocHeader));

		return Ar;
	}

	void MakeMagic()
	{
		memcpy(TocMagic, TocMagicImg, sizeof TocMagic);
	}

	bool CheckMagic() const
	{
		return memcpy((void*)TocMagic, TocMagicImg, sizeof TocMagic);
	}

	__forceinline bool IsEncrypted()
	{
		return EnumHasAnyFlags(ContainerFlags, EIoContainerFlags::Encrypted);
	}
};

struct FIoStoreTocEntryMeta
{
	FIoChunkHash ChunkHash;
	FIoStoreTocEntryMetaFlags Flags;
};

struct FIoOffsetAndLength
{
public:
	FIoOffsetAndLength()
	{
		memset(&OffsetAndLength[0], 0, sizeof(OffsetAndLength));
	}

	inline uint64_t GetOffset() const
	{
		return OffsetAndLength[4]
			| (uint64_t(OffsetAndLength[3]) << 8)
			| (uint64_t(OffsetAndLength[2]) << 16)
			| (uint64_t(OffsetAndLength[1]) << 24)
			| (uint64_t(OffsetAndLength[0]) << 32);
	}

	inline uint64_t GetLength() const
	{
		return OffsetAndLength[9]
			| (uint64_t(OffsetAndLength[8]) << 8)
			| (uint64_t(OffsetAndLength[7]) << 16)
			| (uint64_t(OffsetAndLength[6]) << 24)
			| (uint64_t(OffsetAndLength[5]) << 32);
	}

	inline void SetOffset(uint64_t Offset)
	{
		OffsetAndLength[0] = uint8_t(Offset >> 32);
		OffsetAndLength[1] = uint8_t(Offset >> 24);
		OffsetAndLength[2] = uint8_t(Offset >> 16);
		OffsetAndLength[3] = uint8_t(Offset >> 8);
		OffsetAndLength[4] = uint8_t(Offset >> 0);
	}

	inline void SetLength(uint64_t Length)
	{
		OffsetAndLength[5] = uint8_t(Length >> 32);
		OffsetAndLength[6] = uint8_t(Length >> 24);
		OffsetAndLength[7] = uint8_t(Length >> 16);
		OffsetAndLength[8] = uint8_t(Length >> 8);
		OffsetAndLength[9] = uint8_t(Length >> 0);
	}

	bool IsValid()
	{
		for (size_t i = 0; i < 10; i++)
		{
			if (OffsetAndLength[i] != 0) return true;
		}

		return false;
	}

private:
	uint8_t OffsetAndLength[5 + 5];
};

struct FIoStoreTocResource
{
	FIoStoreTocResource(std::string TocFilePath, EIoStoreTocReadOptions ReadOptions);

	enum { CompressionMethodNameLen = 32 };

	std::filesystem::path TocPath;
	FIoStoreTocHeader Header;
	std::vector<FIoChunkId> ChunkIds;
	std::vector<FIoOffsetAndLength> ChunkOffsetLengths;
	std::vector<int32_t> ChunkPerfectHashSeeds;
	std::vector<int32_t> ChunkIndicesWithoutPerfectHash;
	std::vector<FIoStoreTocCompressedBlockEntry> CompressionBlocks;
	std::vector<std::string> CompressionMethods;
	std::vector<FIoStoreTocEntryMeta> ChunkMetas;
	std::vector<uint8_t> DirectoryIndexBuffer;

	static uint64_t HashChunkIdWithSeed(int32_t Seed, const FIoChunkId& ChunkId);

	__forceinline const std::string& GetBlockCompressionMethod(FIoStoreTocCompressedBlockEntry& Block)
	{
		return CompressionMethods[Block.GetCompressionMethodIndex()];
	}
};