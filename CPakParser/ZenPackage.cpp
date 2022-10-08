#include "ZenPackage.h"

static UObjectPtr IndexToObject(FZenPackageHeaderData& Header, std::vector<FExportObject>& Exports, FPackageObjectIndex Index)
{
	if (Index.IsNull())
		return nullptr;

	if (Index.IsExport())
	{
		return Exports[Index.ToExport()].Object;
	}

	if (Index.IsImport())
	{

	}

	return nullptr;
}

void UZenPackage::ProcessExports(FZenPackageData& PackageData)
{
	PackageData.Exports.resize(PackageData.Header.ExportCount);

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

void UZenPackage::CreateExport(FZenPackageHeaderData& Header, std::vector<FExportObject>& Exports, int32_t LocalExportIndex)
{
	auto& Export = Header.ExportMap[LocalExportIndex];
	auto& ExportObject = Exports[LocalExportIndex];
	auto& Object = ExportObject.Object;
	auto& ObjectName = Header.NameMap.GetName(Export.ObjectName);

	Object = std::make_shared<UObject>();
	Object->SetName(ObjectName);

}