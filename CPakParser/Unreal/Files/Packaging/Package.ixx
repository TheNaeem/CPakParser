module;

#include "Core/Defines.h"

export module CPakParser.Package;

export import CPakParser.Core.UObject;

export typedef TObjectPtr<class UPackage> UPackagePtr;

export class UPackage : public UObject 
{
protected:

	TWeakPtr<class GContext> Context;
	std::vector<UObjectPtr> Exports;

public:

	__forceinline std::vector<UObjectPtr>& GetExports()
	{
		return Exports;
	}

	__forceinline UObjectPtr GetFirstExport()
	{
		return Exports.size() ? Exports[0] : nullptr;
	}

	__forceinline UObjectPtr GetExportByName(std::string InName)
	{
		for (auto&& Export : Exports)
		{ 
			if (Export->GetName() == InName)
				return Export;
		}

		return nullptr;
	}
};