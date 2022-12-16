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
import CPakParser.Serialization.MemoryReader;
import CPakParser.Core.FName;

class FIoExportArchive : public FMemoryReader
{
public:

	FIoExportArchive(uint8_t* InBytes, size_t Size, FZenPackageData& InPackageData, bool bFreeBuffer = false)
		: FMemoryReader(InBytes, Size, bFreeBuffer), PackageData(InPackageData)
	{
	}

	inline virtual FArchive& operator<<(FName& Name) override
	{
		FArchive& Ar = *this;

		uint32_t NameIndex;
		Ar << NameIndex;
		uint32_t Number = 0;
		Ar << Number;

		auto MappedName = FMappedName::Create(NameIndex, Number, FMappedName::EType::Package);

		auto NameStr = PackageData.Header.NameMap.GetName(MappedName);

		if (NameStr.empty())
		{
			LogWarn("Name serialized with FIoExportArchive is empty or invalid");
		}

		Name = NameStr;

		return *this;
	}

	virtual FArchive& operator<<(UObjectPtr& Object) override
	{
		auto& Ar = *this;

		FPackageIndex Index;
		Ar << Index;

		if (Index.IsNull())
		{
			Object = UObjectPtr(nullptr);
			return *this;
		}

		if (Index.IsExport())
		{
			auto ExportIndex = Index.ToExport();
			if (ExportIndex < PackageData.Exports.size())
			{
				Object = PackageData.Exports[ExportIndex].Object;
			}
			else LogError("Export index read is not a valid index.");

			return *this;
		}

		auto& ImportMap = PackageData.Header.ImportMap;

		if (Index.IsImport() && Index.ToImport() < ImportMap.size())
		{
			Object = PackageData.Package->IndexToObject(
				PackageData.Header,
				PackageData.Exports,
				PackageData.Header.ImportMap[Index.ToImport()]);
		}
		else LogError("Bad object import index.");

		return *this;
	}

	FZenPackageData& PackageData;
};

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

std::vector<uint8_t> FIoStoreToc::ReadEntry(FFileEntryInfo& Entry)
{
	std::vector<uint8_t> RetBuf;

	if (Entry.GetTocIndex() >= Toc->ChunkOffsetLengths.size())
	{
		LogError("Invalid entry toc index.");
		return RetBuf;
	}

	auto& OaL = Toc->ChunkOffsetLengths[Entry.GetTocIndex()];

	RetBuf.resize(OaL.GetLength());

	Reader->Read(OaL.GetOffset(), OaL.GetLength(), RetBuf.data());

	return RetBuf;
}

static FZenPackageHeaderData ReadZenPackageHeader(FArchive& Ar, FFileIoStoreContainerFile& Container)
{
	auto PackageHeaderDataPtr = static_cast<uint8_t*>(Ar.Data()); // TODO: fix this crap

	if (!PackageHeaderDataPtr)
	{
		LogWarn("Package header data is null, which either means something went wrong or the archive passed in is not a memory archive. Expect issues.");
	}

	auto PackageDataOffset = Ar.Tell();

	FZenPackageHeaderData Header;
	FZenPackageSummary& Summary = Header.PackageSummary;

	Ar.Serialize(&Header.PackageSummary, sizeof(FZenPackageSummary));

	if (Summary.bHasVersioningInfo)
	{
		Ar.Serialize(&Header.VersioningInfo.ZenVersion, sizeof(EZenPackageVersion));
		Ar << Header.VersioningInfo.PackageVersion;
		Ar << Header.VersioningInfo.LicenseeVersion;

		Header.VersioningInfo.CustomVersions.Serialize(Ar);
	}

	Header.NameMap.Serialize(Ar, FMappedName::EType::Package);
	Header.PackageName = Header.NameMap.GetName(Summary.Name);

	Ar.Seek(PackageDataOffset + Summary.ImportMapOffset);
	Ar.BulkSerializeArray(Header.ImportMap,
		(Summary.ExportMapOffset - Summary.ImportMapOffset) / sizeof(FPackageObjectIndex));

	Ar.Seek(PackageDataOffset + Summary.ExportMapOffset);
	Ar.BulkSerializeArray(Header.ExportMap,
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

		Ar.Seek(PackageDataOffset + Summary.GraphDataOffset);
		Ar.BulkSerializeArray(Header.ExportBundleHeaders, StoreEntry.ExportBundleCount);

		Ar.Seek(PackageDataOffset + Summary.ExportBundleEntriesOffset);
		Ar.BulkSerializeArray(Header.ExportBundleEntries, StoreEntry.ExportCount * FExportBundleEntry::ExportCommandType_Count);
	}

	Header.AllExportDataPtr = PackageHeaderDataPtr + Summary.HeaderSize; // archive being used here should ALWAYS be a memory reader

	return Header;
}

UPackagePtr FIoStoreToc::CreatePackage(FArchive& Ar, TSharedPtr<class GContext> Context) // TODO: micro optimizations
{
	FZenPackageData PackageData;
	PackageData.Header = ReadZenPackageHeader(Ar, Reader->GetContainer());

	auto ExportDataSize = Ar.TotalSize() - (PackageData.Header.AllExportDataPtr - static_cast<uint8_t*>(Ar.Data()));

	TObjectPtr<UZenPackage> Package = PackageData.Package = std::make_shared<UZenPackage>(PackageData.Header, Context);

	PackageData.Reader = std::make_unique<FIoExportArchive>(PackageData.Header.AllExportDataPtr, ExportDataSize, PackageData);
	PackageData.Reader->SetUnversionedProperties(PackageData.HasFlags(PKG_UnversionedProperties));

	Log("Loading package %s", PackageData.Header.PackageName.c_str());

	auto& PackageVer = PackageData.Header.VersioningInfo.PackageVersion;

	if (PackageVer.IsValid())
	{
		PackageData.Reader->SetUEVer(PackageVer);
	}
	else
	{
		PackageData.Reader->SetUEVer(Context->GPackageFileUEVersion);
	}

	Package->ProcessExports(PackageData);

	return Package;
}