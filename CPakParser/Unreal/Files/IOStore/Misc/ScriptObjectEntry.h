#pragma once

#include "Core/Names/MappedName.h"
#include "Files/Packaging/PackageIndex.h"

struct FScriptObjectEntry
{
	FMappedName MappedName;
	FPackageObjectIndex GlobalIndex;
	FPackageObjectIndex OuterIndex;
	FPackageObjectIndex CDOClassIndex;
};