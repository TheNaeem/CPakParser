#include "ZenPackage.h"

#include "LazyPackageObject.h"
#include "Class.h"

template <typename T = UObject>
TObjectPtr<T> CreateScriptObject(TSharedPtr<GContext> Context, FPackageObjectIndex& Index)
{
	auto ScriptObject = Context->GlobalToc.ScriptObjectByGlobalIdMap[Index];
	auto Name = Context->GlobalToc.NameMap.GetName(ScriptObject.MappedName);

	auto Ret = std::make_shared<T>();
	Ret->SetName(Name);

	if (Context->ObjectArray.contains(Name))
	{
		return Context->ObjectArray[Name];
	}

	if (!ScriptObject.OuterIndex.IsNull())
	{
		Ret->SetOuter(CreateScriptObject<UObject>(Context, ScriptObject.OuterIndex));
	}

	if (!ScriptObject.CDOClassIndex.IsNull())
	{
		Ret->SetClass(CreateScriptObject<UClass>(Context, ScriptObject.OuterIndex));
	}

	Ret->SetFlags(RF_NeedLoad);

	return Ret;
}

template <typename T>
UObjectPtr UZenPackage::IndexToObject(FZenPackageHeaderData& Header, std::vector<FExportObject>& Exports, FPackageObjectIndex Index)
{
	if (Index.IsNull())
		return TObjectPtr<T>();

	if (Index.IsExport())
	{
		return Exports[Index.ToExport()].Object;
	}

	if (Index.IsImport())
	{
		if (Index.IsScriptImport())
		{
			auto Ret = CreateScriptObject<T>(Context, Index);

			if (!Context->ObjectArray.contains(Ret->GetName()))
				Context->ObjectArray.insert_or_assign(Ret->GetName(), Ret);

			return Ret;
		}
		else if (Index.IsPackageImport())
		{
			auto PackageId = Header.ImportedPackageIds[Index.GetImportedPackageIndex()];

			return UObjectPtr(std::make_shared<ULazyPackageObject>(PackageId));
		}
	}

	return TObjectPtr<T>();
}

void UZenPackage::ProcessExports(FZenPackageData& PackageData)
{
	PackageData.Exports.resize(PackageData.Header.ExportCount);

	for (size_t i = 0; i < PackageData.Exports.size(); i++)
	{
		if (!PackageData.Exports[i].Object)
			PackageData.Exports[i].Object = std::make_shared<UObject>();
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
				continue;
			}

			if (BundleEntry.CommandType != FExportBundleEntry::ExportCommandType_Serialize)
				continue;

			SerializeExport(PackageData, BundleEntry.LocalExportIndex);
		}
	}
}

void UZenPackage::CreateExport(FZenPackageHeaderData& Header, std::vector<FExportObject>& Exports, int32_t LocalExportIndex) // TODO: make it return object of passed in type
{
	auto& Export = Header.ExportMap[LocalExportIndex];
	UObjectPtr& Object = Exports[LocalExportIndex].Object;
	auto& TemplateObject = Exports[LocalExportIndex].TemplateObject;
	auto& ObjectName = Header.NameMap.GetName(Export.ObjectName);
	
	TemplateObject = IndexToObject(Header, Exports, Export.TemplateIndex);

	if (!TemplateObject)
	{
		Log<Error>("Template object could not be loaded for zen package.");
		return;
	}

	if (Context->ObjectArray.contains(ObjectName))
	{
		Object = Context->ObjectArray[ObjectName];
		return;
	}

	Object->Name = ObjectName;

	if (!Object->Class)
		Object->Class = IndexToObject<UClass>(Header, Exports, Export.ClassIndex).As<UClass>();

	if (!Object->Outer)
		Object->Outer = Export.OuterIndex.IsNull() ? UObjectPtr(shared_from_this()) : IndexToObject(Header, Exports, Export.OuterIndex);
	 
	if (UStructPtr Struct = Object.As<UStruct>())
	{
		if (!Struct->GetSuper())
			Struct->SetSuper(IndexToObject<UStruct>(Header, Exports, Export.SuperIndex).As<UStruct>());
	}

	Object->Flags = EObjectFlags(Export.ObjectFlags | RF_NeedLoad | RF_NeedPostLoad | RF_NeedPostLoadSubobjects | RF_WasLoaded);
}

void UZenPackage::SerializeExport(FZenPackageData& PackageData, int32_t LocalExportIndex)
{
	auto& Export = PackageData.Header.ExportMap[LocalExportIndex];
	auto& ExportObject = PackageData.Exports[LocalExportIndex];
	UObjectPtr& Object = ExportObject.Object;

	// TODO: struct casting maybe? idk

	Object->ClearFlags(RF_NeedLoad);

	/*if (Object->HasAnyFlags(RF_ClassDefaultObject)) // TODO
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(SerializeDefaultObject);
		Object->GetClass()->SerializeDefaultObject(Object, Ar);
	}
	else*/

	PackageData.Reader->SetArchetype(ExportObject.TemplateObject);

	Object->Serialize(PackageData.Reader);

	PackageData.Reader->SetArchetype({});
}