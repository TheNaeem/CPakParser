#include "AssetObject.h"

#include "UObject.h"
#include "ZenData.h"

UAssetObject::UAssetObject(FGlobalTocData& GlobalToc, FPackageObjectIndex& Index)
{
	auto ScriptObject = GlobalToc.ScriptObjectByGlobalIdMap[Index]; 
	Name = GlobalToc.NameMap.GetName(ScriptObject.MappedName);

	if (!ScriptObject.OuterIndex.IsNull())
	{
		Outer = std::make_shared<UAssetObject>(GlobalToc, ScriptObject.OuterIndex);
	}

	if (!ScriptObject.CDOClassIndex.IsNull())
	{
		Class = std::make_shared<UAssetObject>(GlobalToc, ScriptObject.OuterIndex);
	}
}