#pragma once

#include "../Container/IoContainerId.h"
#include "Misc/Guid.h"
#include "Core/Defines.h"
#include "TocCompressedBlock.h"
#include "Misc/Hashing/IoChunkHash.h"
#include <vector>
#include <string>

enum class EIoStoreTocReadOptions
{
	Default,
	ReadDirectoryIndex = (1 << 0),
	ReadTocMeta = (1 << 1),
	ReadAll = ReadDirectoryIndex | ReadTocMeta
};

enum class EIoContainerFlags : uint8_t
{
	None,
	Compressed = (1 << 0),
	Encrypted = (1 << 1),
	Signed = (1 << 2),
	Indexed = (1 << 3),
};
ENUM_CLASS_FLAGS(EIoContainerFlags);

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

	bool CheckMagic() const
	{
		return memcpy((void*)TocMagic, TocMagicImg, sizeof TocMagic);
	}

	bool IsEncrypted();
};

struct FIoStoreTocEntryMeta
{
	FIoChunkHash ChunkHash;
	uint8_t Flags;
};

struct FIoStoreTocResource
{
	FIoStoreTocResource(std::string TocFilePath, EIoStoreTocReadOptions ReadOptions);

	enum { CompressionMethodNameLen = 32 };

	std::string TocPath;
	FIoStoreTocHeader Header;
	std::vector<class FIoChunkId> ChunkIds;
	std::vector<struct FIoOffsetAndLength> ChunkOffsetLengths;
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