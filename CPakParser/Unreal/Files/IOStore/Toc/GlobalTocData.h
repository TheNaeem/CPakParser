#pragma once

#include "Core/Defines.h"
#include "Core/Names/NameMap.h"
#include "Misc/Hashing/Map.h"
#include "Files/Packaging/PackageIndex.h"
#include "Files/IOStore/Misc/ScriptObjectEntry.h"

struct FGlobalTocData // inspired by the CUE4Parse IoGlobalData class. huge shoutout to them
{
	FNameMap NameMap;
	TMap<class FPackageObjectIndex, class FScriptObjectEntry> ScriptObjectByGlobalIdMap;

	void Serialize(TSharedPtr<class FIoStoreReader> Reader);
};