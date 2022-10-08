#pragma once

#include "Package.h"
#include "ZenData.h"

struct FZenPackageData
{
	FSharedAr Reader;
	FZenPackageHeaderData Header;
	std::vector<FExportObject> Exports;

	__forceinline bool HasFlags(uint32_t Flags)
	{
		return Header.HasFlags(Flags);
	}
};

class UZenPackage : public UPackage
{
public:

	UZenPackage(FZenPackageHeaderData& InHeader) 
	{
		Name = InHeader.PackageName;
	}

	void ProcessExports(FZenPackageData& PackageData);
	void CreateExport(FZenPackageHeaderData& Header, std::vector<FExportObject>& Exports, int32_t LocalExportIndex);
};