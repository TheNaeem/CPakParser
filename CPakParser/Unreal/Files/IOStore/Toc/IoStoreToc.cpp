#include "IoStoreToc.h"
#include "Core/Globals/GlobalContext.h"
#include "../Misc/IoStoreReader.h"

import CPakParser.IOStore.TocResource;
import CPakParser.Package.Index;
import CPakParser.Logging;
import CPakParser.Files.FileEntry;
import CPakParser.Package.Flags;
import CPakParser.Zen.Package;
import CPakParser.Zen.Data;
import CPakParser.Core.FName;
import CPakParser.Serialization.MemoryReader;

FIoStoreToc::FIoStoreToc(std::string& TocFilePath)
	: FIoStoreToc(std::make_shared<FIoStoreTocResource>(TocFilePath, EIoStoreTocReadOptions::ReadAll))
{
}

FIoStoreToc::FIoStoreToc(TSharedPtr<FIoStoreTocResource> TocRsrc) : Toc(TocRsrc)
{
	ChunkIdToIndex.reserve(Toc->ChunkIds.size());

	for (size_t i = 0; i < Toc->ChunkIds.size(); i++)
	{
		ChunkIdToIndex.insert_or_assign(Toc->ChunkIds[i], i);
	}
}

FAESKey& FIoStoreToc::GetEncryptionKey()
{
	return Key;
}

TSharedPtr<FIoStoreTocResource> FIoStoreToc::GetResource()
{
	return Toc;
}

std::string FIoStoreToc::GetDiskPath()
{
	return Toc->TocPath;
}

void FIoStoreToc::SetKey(FAESKey& InKey)
{
	Key = InKey;
}

void FIoStoreToc::SetReader(TSharedPtr<class FIoStoreReader> InReader)
{
	Reader = InReader;
}

int32_t FIoStoreToc::GetTocEntryIndex(struct FIoChunkId& ChunkId)
{
	return ChunkIdToIndex[ChunkId];
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

static FZenPackageHeaderData ReadZenPackageHeader(FSharedAr Ar, FFileIoStoreContainerFile& Container)
{
	auto PackageHeaderDataPtr = static_cast<uint8_t*>(Ar->Data()); // TODO: fix this crap

	if (!PackageHeaderDataPtr)
	{
		LogWarn("Package header data is null, which either means something went wrong or the archive passed in is not a memory archive. Expect issues.");
	}

	auto PackageDataOffset = Ar->Tell();

	FZenPackageHeaderData Header;
	FZenPackageSummary& Summary = Header.PackageSummary;

	Ar->Serialize(&Header.PackageSummary, sizeof(FZenPackageSummary));

	if (Summary.bHasVersioningInfo)
	{
		Ar->Serialize(&Header.VersioningInfo.ZenVersion, sizeof(EZenPackageVersion));
		*Ar << Header.VersioningInfo.PackageVersion;
		*Ar << Header.VersioningInfo.LicenseeVersion;

		Header.VersioningInfo.CustomVersions.Serialize(*Ar);
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

	if (Container.Header.PackageStore.contains(PackageId))
	{
		auto& StoreEntry = Container.Header.PackageStore[PackageId];

		Header.ExportCount = StoreEntry.ExportCount;

		auto ImportedPackagesCount = StoreEntry.ImportedPackages.Num();

		Header.ImportedPackageIds.resize(ImportedPackagesCount);
		memcpy(Header.ImportedPackageIds.data(), StoreEntry.ImportedPackages.Data(), sizeof(FPackageId) * ImportedPackagesCount);

		Ar->Seek(PackageDataOffset + Summary.GraphDataOffset);
		Ar->BulkSerializeArray(Header.ExportBundleHeaders, StoreEntry.ExportBundleCount);

		Ar->Seek(PackageDataOffset + Summary.ExportBundleEntriesOffset);
		Ar->BulkSerializeArray(Header.ExportBundleEntries, StoreEntry.ExportCount * FExportBundleEntry::ExportCommandType_Count);
	}

	Header.AllExportDataPtr = PackageHeaderDataPtr + Summary.HeaderSize; // archive being used here should ALWAYS be a memory reader // TODO: this scuffed crap

	return Header;
}

void FIoStoreToc::DoWork(FSharedAr Ar, TSharedPtr<GContext> Context) // TODO: pass in loader. TODO: micro optimizations
{
	FZenPackageData PackageData;

	PackageData.Header = ReadZenPackageHeader(Ar, Reader->GetContainer());
	PackageData.ExportDataSize = Ar->TotalSize() - (PackageData.Header.AllExportDataPtr - static_cast<uint8_t*>(Ar->Data()));

	auto Package = PackageData.Package = std::make_shared<UZenPackage>(PackageData.Header, Context);

	Log("Loading package %s", PackageData.Header.PackageName.c_str());

	Package->ProcessExports(PackageData);
}