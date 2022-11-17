export module CPakParser.IOStore.ScriptObjectEntry;

import CPakParser.Names.MappedName;
import CPakParser.Package.ObjectIndex;

export struct FScriptObjectEntry
{
	FMappedName MappedName;
	FPackageObjectIndex GlobalIndex;
	FPackageObjectIndex OuterIndex;
	FPackageObjectIndex CDOClassIndex;
};