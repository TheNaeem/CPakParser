#pragma once

#include "CoreTypes.h"

struct FGlobalTocData // inspired by the CUE4Parse IoGlobalData class. huge shoutout to them
{
	class FNameMap NameMap;
	phmap::flat_hash_map<class FPackageObjectIndex, class FScriptObjectEntry> ScriptObjectByGlobalIdMap;

	void Serialize(TSharedPtr<class FIoStoreReader> Reader);
};