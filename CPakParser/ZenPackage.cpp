#include "ZenPackage.h"

UObjectPtr UZenPackage::IndexToObject(FZenPackageHeaderData& Header, std::vector<UObjectPtr>& Exports, FPackageObjectIndex Index)
{
	if (Index.IsNull())
		return UObjectPtr();

	if (Index.IsExport())
	{
		return Exports[Index.ToExport()];
	}

	if (Index.IsImport())
	{
		if (Index.IsScriptImport())
		{
			return UObjectPtr(std::make_shared<UAssetObject>(Context->GlobalToc, Index));
		}
	}

	return UObjectPtr();
}

void UZenPackage::ProcessExports(FZenPackageData& PackageData)
{
	PackageData.Exports.resize(PackageData.Header.ExportCount);

	for (size_t i = 0; i < PackageData.Exports.size(); i++)
	{
		if (!PackageData.Exports[i])
			PackageData.Exports[i] = std::make_shared<UObject>();
	}

	auto& Header = PackageData.Header;

	for (size_t i = 0; i < Header.ExportBundleHeaders.size(); i++)
	{
		// TODO: Ar.SetUEVer(Package->LinkerRoot->GetLinkerPackageVersion());

		auto& ExportBundle = Header.ExportBundleHeaders[i];

		for (size_t ExportBundleIndex = 0; ExportBundleIndex < ExportBundle.EntryCount; ExportBundleIndex++)
		{
			const auto& BundleEntry = Header.ExportBundleEntries[ExportBundle.FirstEntryIndex + ExportBundleIndex];
			const auto& ExportMapEntry = Header.ExportMap[BundleEntry.LocalExportIndex];

			if (BundleEntry.CommandType == FExportBundleEntry::ExportCommandType_Create)
			{
				CreateExport(Header, PackageData.Exports, BundleEntry.LocalExportIndex);
			}
		}
	}
}

void UZenPackage::CreateExport(FZenPackageHeaderData& Header, std::vector<UObjectPtr>& Exports, int32_t LocalExportIndex)
{
	auto& Export = Header.ExportMap[LocalExportIndex];
	auto& Object = Exports[LocalExportIndex];
	auto& ObjectName = Header.NameMap.GetName(Export.ObjectName);

	Object->Name = ObjectName;

	if (!Object->Class)
		Object->Class = IndexToObject(Header, Exports, Export.ClassIndex);

	if (!Object->Outer)
		Object->Outer = Export.OuterIndex.IsNull() ? UObjectPtr(shared_from_this()) : IndexToObject(Header, Exports, Export.OuterIndex);

	if (!Object->Super)
		Object->Super = IndexToObject(Header, Exports, Export.SuperIndex);


}