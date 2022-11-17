module;

#include "Core/Defines.h"

export module CPakParser.Zen.Package;

export import CPakParser.Core.UObject;
export import CPakParser.Package;
import CPakParser.Serialization.FArchive;
import CPakParser.Zen.Data;
import <vector>;

export struct FExportObject
{
	UObjectPtr Object;
	UObjectPtr TemplateObject;
};

export struct FZenPackageData
{
	TSharedPtr<class UZenPackage> Package;
	FSharedAr Reader;
	FZenPackageHeaderData Header;
	std::vector<FExportObject> Exports;

	__forceinline bool HasFlags(uint32_t Flags)
	{
		return Header.HasFlags(Flags);
	}
};

export class UZenPackage : public UPackage
{
public:

	UZenPackage(FZenPackageHeaderData& InHeader, TSharedPtr<GContext> InContext)
	{
		Name = InHeader.PackageName;
		Context = InContext;
	}

	void ProcessExports(FZenPackageData& PackageData);
	void CreateExport(class FZenPackageHeaderData& Header, std::vector<FExportObject>& Exports, int32_t LocalExportIndex);
	void SerializeExport(FZenPackageData& PackageData, int32_t LocalExportIndex);

	template <typename T = UObject>
	UObjectPtr IndexToObject(FZenPackageHeaderData& Header, std::vector<FExportObject>& Exports, FPackageObjectIndex Index);
};