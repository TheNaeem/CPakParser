#include "IoStoreReader.h"

#include "Compression.h"
#include "MemoryReader.h"
#include "ZenPackage.h"

FIoStoreToc::FIoStoreToc(TSharedPtr<FIoStoreTocResource> TocRsrc) : Toc(TocRsrc)
{
}

FSharedAr FIoStoreToc::CreateEntryArchive(FFileEntryInfo EntryInfo) // TODO: make this async; look into StartAsyncRead
{
	if (EntryInfo.GetTocIndex() >= Toc->ChunkOffsetLengths.size())
		return nullptr;

	auto& OaL = Toc->ChunkOffsetLengths[EntryInfo.GetTocIndex()];

	auto Offset = OaL.GetOffset();
	auto Len = OaL.GetLength();
	auto CompressionBlockSize = Toc->Header.CompressionBlockSize;

	auto FirstBlockIndex = Offset / CompressionBlockSize;
	auto LastBlockIndex = (Align(Offset + Len, CompressionBlockSize) - 1) / CompressionBlockSize;
	auto BlockCount = LastBlockIndex - FirstBlockIndex + 1;

	if (!BlockCount)
		return nullptr;

	auto& FirstBlock = Toc->CompressionBlocks[FirstBlockIndex];
	auto& LastBlock = Toc->CompressionBlocks[LastBlockIndex];

	auto PartitionIndex = static_cast<int32_t>(FirstBlock.GetOffset() / Toc->Header.PartitionSize);

	auto ReadStartOffset = FirstBlock.GetOffset() % Toc->Header.PartitionSize;
	auto ReadEndOffset = (LastBlock.GetOffset() + Align(LastBlock.GetCompressedSize(), FAESKey::AESBlockSize)) % Toc->Header.PartitionSize;

	auto CompressedSize = ReadEndOffset - ReadStartOffset;

	auto DecompressionBuf = static_cast<uint8_t*>(malloc(Len));
	auto CompressedBuf = std::make_unique<uint8_t[]>(CompressedSize);

	Reader->Read(PartitionIndex, ReadStartOffset, CompressedSize, CompressedBuf.get());

	uint64_t CompressedOffset = 0;
	uint64_t DecompressedOffset = 0;
	uint64_t OffsetInBlock = Offset % CompressionBlockSize;
	uint64_t RemainingSize = Len;

	for (auto i = FirstBlockIndex; i <= LastBlockIndex; ++i)
	{
		auto CompressedData = CompressedBuf.get() + CompressedOffset;
		auto DecompressedData = DecompressionBuf + CompressedOffset;

		auto& CompressionBlock = Toc->CompressionBlocks[i];

		auto BlockCompressedSize = Align(CompressionBlock.GetCompressedSize(), FAESKey::AESBlockSize);
		auto UncompressedSize = CompressionBlock.GetUncompressedSize();

		auto& CompressionMethod = Toc->GetBlockCompressionMethod(CompressionBlock);

		if (Toc->Header.IsEncrypted())
		{
			Key.DecryptData(CompressedData, BlockCompressedSize);
		}

		if (CompressionMethod.empty())
		{
			memcpy(DecompressedData, CompressedData + OffsetInBlock, UncompressedSize - OffsetInBlock);
		}
		else if (OffsetInBlock || RemainingSize < UncompressedSize)
		{
			// TODO: is this really necessary? give this another look
			std::vector<uint8_t> TempBuffer(UncompressedSize);
			FCompression::DecompressMemory(CompressionMethod, TempBuffer.data(), UncompressedSize, CompressedData, CompressionBlock.GetCompressedSize());

			auto CopySize = std::min(UncompressedSize - OffsetInBlock, RemainingSize);
			memcpy(DecompressedData, TempBuffer.data() + OffsetInBlock, CopySize);
		}
		else
		{
			FCompression::DecompressMemory(CompressionMethod, DecompressedData, UncompressedSize, CompressedData, BlockCompressedSize);
		}

		CompressedOffset += BlockCompressedSize;
		DecompressedOffset += UncompressedSize;
		RemainingSize -= UncompressedSize;
		OffsetInBlock = 0;
	}

	return std::make_shared<FMemoryReader>(DecompressionBuf, Len, true);
}

FZenPackageHeaderData FIoStoreReader::ReadZenPackageHeader(FSharedAr Ar)
{
	auto PackageDataOffset = Ar->Tell();

	FZenPackageHeaderData Header;
	FZenPackageSummary& Summary = Header.PackageSummary;

	Ar->Serialize(&Header.PackageSummary, sizeof(FZenPackageSummary));

	if (Summary.bHasVersioningInfo)
	{
		*Ar << Header.VersioningInfo;
	}

	Header.NameMap.Serialize(*Ar, FMappedName::EType::Package);
	Header.PackageName = Header.NameMap.GetName(Summary.Name);

	Ar->Seek(PackageDataOffset + Summary.ImportMapOffset);
	Ar->BulkSerializeArray(Header.ImportMap,
		(Summary.ExportMapOffset - Summary.ImportMapOffset) / sizeof(FPackageObjectIndex));

	Ar->Seek(PackageDataOffset + Summary.ExportMapOffset);
	Ar->BulkSerializeArray(Header.ExportMap,
		(Summary.ExportBundleEntriesOffset - Summary.ExportMapOffset) / sizeof(FExportMapEntry));

	// TODO: do we really need the arcs data?

	auto PackageId = FPackageId(Header.PackageName);

	auto& Container = GetContainer();

	if (Container.Header.PackageStore.contains(PackageId))
	{
		auto& StoreEntry = Container.Header.PackageStore[PackageId];

		Header.ExportCount = StoreEntry.ExportCount;

		Ar->Seek(PackageDataOffset + Summary.GraphDataOffset);
		Ar->BulkSerializeArray(Header.ExportBundleHeaders, StoreEntry.ExportBundleCount);

		Ar->Seek(PackageDataOffset + Summary.ExportBundleEntriesOffset);
		Ar->BulkSerializeArray(Header.ExportBundleEntries, StoreEntry.ExportCount * FExportBundleEntry::ExportCommandType_Count);
	}

	return Header;
}

void FIoStoreToc::DoWork(FSharedAr Ar) // TODO: pass in loader
{
	FZenPackageData PackageData;

	PackageData.Reader = Ar;
	PackageData.Header = Reader->ReadZenPackageHeader(Ar);

	auto Package = std::make_shared<UZenPackage>(PackageData.Header);

	Ar->SetUnversionedProperties(PackageData.HasFlags(PKG_UnversionedProperties));

	Package->ProcessExports(PackageData);
}