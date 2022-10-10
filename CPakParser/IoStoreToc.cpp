#include "IoStoreReader.h"

#include "Compression.h"
#include "MemoryReader.h"
#include "ZenPackage.h"

FIoStoreToc::FIoStoreToc(TSharedPtr<FIoStoreTocResource> TocRsrc) : Toc(TocRsrc)
{
	ChunkIdToIndex.reserve(Toc->ChunkIds.size());

	for (size_t i = 0; i < Toc->ChunkIds.size(); i++)
	{
		ChunkIdToIndex.insert_or_assign(Toc->ChunkIds[i], i);
	}
}

FIoOffsetAndLength FIoStoreToc::GetOffsetAndLength(FIoChunkId& ChunkId) 
{
	if (!ChunkIdToIndex.contains(ChunkId))
		return FIoOffsetAndLength();

	auto Index = ChunkIdToIndex[ChunkId];

	return Toc->ChunkOffsetLengths[Index];
}

FSharedAr FIoStoreToc::CreateEntryArchive(FFileEntryInfo EntryInfo) 
{
	if (EntryInfo.GetTocIndex() >= Toc->ChunkOffsetLengths.size())
		return nullptr;

	auto& OaL = Toc->ChunkOffsetLengths[EntryInfo.GetTocIndex()];

	return std::make_shared<FMemoryReader>(Reader->Read(OaL).release(), OaL.GetLength(), true);
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

void FIoStoreToc::DoWork(FSharedAr Ar, TSharedPtr<GContext> Context) // TODO: pass in loader
{
	FZenPackageData PackageData;

	PackageData.Reader = Ar;
	PackageData.Header = Reader->ReadZenPackageHeader(Ar);

	auto Package = std::make_shared<UZenPackage>(PackageData.Header, Context);

	Ar->SetUnversionedProperties(PackageData.HasFlags(PKG_UnversionedProperties));

	Package->ProcessExports(PackageData);
}