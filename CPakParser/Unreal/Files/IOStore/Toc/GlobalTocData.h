#pragma once

#include "Core/Defines.h"
#include "Misc/Hashing/Map.h"

import CPakParser.IOStore.ScriptObjectEntry;
import CPakParser.Names.NameMap;
import CPakParser.Package.ObjectIndex;

export struct FGlobalTocData // inspired by the CUE4Parse IoGlobalData class. huge shoutout to them
{
	FNameMap NameMap;
	TMap<FPackageObjectIndex, FScriptObjectEntry> ScriptObjectByGlobalIdMap;

	void Serialize(TSharedPtr<class FIoStoreReader> Reader);
};