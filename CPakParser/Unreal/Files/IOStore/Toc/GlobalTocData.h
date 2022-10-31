#pragma once

#include "Core/Defines.h"
#include "Core/Names/NameMap.h"
#include "Misc/Hashing/Map.h"
#include "Files/Packaging/PackageObjectIndex.h"
#include "Files/IOStore/Misc/ScriptObjectEntry.h"

struct FGlobalTocData // inspired by the CUE4Parse IoGlobalData class. huge shoutout to them
{
	FNameMap NameMap;
	TMap<FPackageObjectIndex, FScriptObjectEntry> ScriptObjectByGlobalIdMap;

	void Serialize(TSharedPtr<class FIoStoreReader> Reader);
};