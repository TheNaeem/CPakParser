#include "IoTocResource.h"
#include "MemoryReader.h"
#include "FileReader.h"

FIoStoreTocResource::FIoStoreTocResource(std::string TocFilePath, EIoStoreTocReadOptions ReadOptions)
{
	this->TocPath = TocFilePath;

	auto TocFileReader = FFileReader(TocFilePath.c_str());

	if (!TocFileReader.IsValid())
	{
		ReadStatus(ReadErrorCode::FileOpenFailed, "Failed to open IoStore TOC file: " + TocFilePath);
		return;
	}

	TocFileReader << Header;

	if (!Header.CheckMagic())
	{
		ReadStatus(ReadErrorCode::CorruptFile, "Could not read TOC file magic: " + TocFilePath);
		return;
	}

	if (Header.TocHeaderSize != sizeof(FIoStoreTocHeader))
		ReadStatus(ReadErrorCode::CorruptFile, "User defined FIoStoreTocHeader is not the same size as the one used in the TOC");


	if (Header.TocCompressedBlockEntrySize != sizeof(FIoStoreTocCompressedBlockEntry))
		ReadStatus(ReadErrorCode::CorruptFile, "User defined FIoStoreTocCompressedBlockEntry is not the same size as the one used in the TOC");

	uint64_t TotalTocSize = TocFileReader.TotalSize() - sizeof(FIoStoreTocHeader);
	uint64_t TocMetaSize = Header.TocEntryCount * sizeof(FIoStoreTocEntryMeta);
	uint64_t DefaultTocSize = TotalTocSize - Header.DirectoryIndexSize - TocMetaSize;
	uint64_t TocSize = DefaultTocSize;

	if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadTocMeta))
	{
		TocSize = TotalTocSize;
	}
	else if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadDirectoryIndex))
	{
		TocSize = DefaultTocSize + Header.DirectoryIndexSize;
	}

	auto TocBuffer = std::make_unique<uint8_t[]>(TocSize);
	TocFileReader.Serialize(TocBuffer.get(), TocSize);

	auto TocMemReader = FMemoryReader(TocBuffer.get(), TocSize);

	TocMemReader.BulkSerializeArray(ChunkIds, Header.TocEntryCount);
	TocMemReader.BulkSerializeArray(ChunkOffsetLengths, Header.TocEntryCount);

	uint32_t PerfectHashSeedsCount = 0;
	uint32_t ChunksWithoutPerfectHashCount = 0;

	if (Header.Version >= static_cast<uint8_t>(EIoStoreTocVersion::PerfectHashWithOverflow))
	{
		PerfectHashSeedsCount = Header.TocChunkPerfectHashSeedsCount;
		ChunksWithoutPerfectHashCount = Header.TocChunksWithoutPerfectHashCount;
	}
	else if (Header.Version >= static_cast<uint8_t>(EIoStoreTocVersion::PerfectHash))
	{
		PerfectHashSeedsCount = Header.TocChunkPerfectHashSeedsCount;
	}

	if (PerfectHashSeedsCount)
	{
		TocMemReader.BulkSerializeArray(ChunkPerfectHashSeeds, PerfectHashSeedsCount);
	}

	if (ChunksWithoutPerfectHashCount)
	{
		TocMemReader.BulkSerializeArray(ChunkIndicesWithoutPerfectHash, ChunksWithoutPerfectHashCount);
	}

	TocMemReader.BulkSerializeArray(CompressionBlocks, Header.TocCompressedBlockEntryCount);

	CompressionMethods.reserve(Header.CompressionMethodNameCount + 1);
	CompressionMethods.push_back({});

	for (auto CompressonNameIndex = 0; CompressonNameIndex < Header.CompressionMethodNameCount; CompressonNameIndex++)
	{
		auto AnsiCompressionMethodName = reinterpret_cast<const char*>(TocMemReader.GetBufferCur()) + CompressonNameIndex * Header.CompressionMethodNameLength;
		CompressionMethods.push_back(AnsiCompressionMethodName);
	}

	TocMemReader.SeekCur(Header.CompressionMethodNameCount * Header.CompressionMethodNameLength);

	bool bIsSigned = EnumHasAnyFlags(Header.ContainerFlags, EIoContainerFlags::Signed);
	if (bIsSigned)
	{
		uint32_t HashSize;
		TocMemReader << HashSize;

		TocMemReader.SeekCur(HashSize + HashSize + (sizeof(FSHAHash) * Header.TocCompressedBlockEntryCount));

		//TODO: get chunk block signatures
	}

	if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadDirectoryIndex) &&
		EnumHasAnyFlags(Header.ContainerFlags, EIoContainerFlags::Indexed) &&
		Header.DirectoryIndexSize > 0)
	{
		auto Buf = TocMemReader.GetBufferCur();

		this->DirectoryIndexBuffer = std::vector<uint8_t>(Buf, Buf + Header.DirectoryIndexSize);
	}

	TocMemReader.SeekCur(Header.DirectoryIndexSize);
	if (EnumHasAnyFlags(ReadOptions, EIoStoreTocReadOptions::ReadTocMeta))
	{
		auto Buf = (FIoStoreTocEntryMeta*)TocMemReader.GetBufferCur();

		ChunkMetas = std::vector<FIoStoreTocEntryMeta>(Buf, Buf + Header.TocEntryCount);
	}

	if (Header.Version < static_cast<uint8_t>(EIoStoreTocVersion::PartitionSize))
	{
		Header.PartitionCount = 1;
		Header.PartitionSize = MAX_uint64;
	}

	ReadStatus(ReadErrorCode::Ok, "Successfully processed TOC header: " + TocFilePath);
}

uint64_t FIoStoreTocResource::HashChunkIdWithSeed(int32_t Seed, const FIoChunkId& ChunkId)
{
	const uint8_t* Data = ChunkId.GetData();
	const uint32_t DataSize = ChunkId.GetSize();
	uint64_t Hash = Seed ? static_cast<uint64_t>(Seed) : 0xcbf29ce484222325;

	for (uint32_t Index = 0; Index < DataSize; ++Index)
	{
		Hash = (Hash * 0x00000100000001B3) ^ Data[Index];
	}

	return Hash;
}