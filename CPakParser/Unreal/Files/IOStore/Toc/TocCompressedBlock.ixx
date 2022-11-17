export module CPakParser.IOStore.TocCompressedBlock;

import <cstdint>;

export struct FIoStoreTocCompressedBlockEntry
{
	static constexpr uint32_t OffsetBits = 40;
	static constexpr uint64_t OffsetMask = (1ull << OffsetBits) - 1ull;
	static constexpr uint32_t SizeBits = 24;
	static constexpr uint32_t SizeMask = (1 << SizeBits) - 1;
	static constexpr uint32_t SizeShift = 8;

	__forceinline uint64_t GetOffset() const
	{
		const uint64_t* Offset = reinterpret_cast<const uint64_t*>(Data);
		return *Offset & OffsetMask;
	}

	__forceinline void SetOffset(uint64_t InOffset)
	{
		uint64_t* Offset = reinterpret_cast<uint64_t*>(Data);
		*Offset = InOffset & OffsetMask;
	}

	__forceinline uint32_t GetCompressedSize() const
	{
		const uint32_t* Size = reinterpret_cast<const uint32_t*>(Data) + 1;
		return (*Size >> SizeShift) & SizeMask;
	}

	__forceinline void SetCompressedSize(uint32_t InSize)
	{
		uint32_t* Size = reinterpret_cast<uint32_t*>(Data) + 1;
		*Size |= (uint32_t(InSize) << SizeShift);
	}

	__forceinline uint32_t GetUncompressedSize() const
	{
		const uint32_t* UncompressedSize = reinterpret_cast<const uint32_t*>(Data) + 2;
		return *UncompressedSize & SizeMask;
	}

	__forceinline void SetUncompressedSize(uint32_t InSize)
	{
		uint32_t* UncompressedSize = reinterpret_cast<uint32_t*>(Data) + 2;
		*UncompressedSize = InSize & SizeMask;
	}

	__forceinline uint8_t GetCompressionMethodIndex() const
	{
		const uint32_t* Index = reinterpret_cast<const uint32_t*>(Data) + 2;
		return static_cast<uint8_t>(*Index >> SizeBits);
	}

	__forceinline void SetCompressionMethodIndex(uint8_t InIndex)
	{
		uint32_t* Index = reinterpret_cast<uint32_t*>(Data) + 2;
		*Index |= uint32_t(InIndex) << SizeBits;
	}

private:

	/* 5 bytes offset, 3 bytes for size / uncompressed size and 1 byte for compression method. */
	uint8_t Data[5 + 3 + 3 + 1];
};