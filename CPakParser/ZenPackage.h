#pragma once

#include "Package.h"
#include "ZenData.h"
#include "GlobalContext.h"

struct FZenPackageData
{
	FSharedAr Reader;
	FZenPackageHeaderData Header;
	std::vector<UObjectPtr> Exports;

	__forceinline bool HasFlags(uint32_t Flags)
	{
		return Header.HasFlags(Flags);
	}
};

class UZenPackage : public UPackage, public std::enable_shared_from_this<UZenPackage>
{
	UObjectPtr IndexToObject(FZenPackageHeaderData& Header, std::vector<UObjectPtr>& Exports, FPackageObjectIndex Index);

public:

	UZenPackage(FZenPackageHeaderData& InHeader, TSharedPtr<GContext> InContext)
	{
		Name = InHeader.PackageName;
		Context = InContext;
	}

	void ProcessExports(FZenPackageData& PackageData);
	void CreateExport(FZenPackageHeaderData& Header, std::vector<UObjectPtr>& Exports, int32_t LocalExportIndex);
};