#pragma once

#include "Core/Names/Name.h"

class FTopLevelAssetPath
{
public:

	FTopLevelAssetPath() = default;
	FTopLevelAssetPath(FName& Name);

	__forceinline std::string GetPackageName() const { return PackageName.ToString(); }
	__forceinline std::string GetAssetName() const { return AssetName.ToString(); }

	friend class FArchive& operator<<(class FArchive& Ar, FTopLevelAssetPath& Value);

	std::string ToString()
	{
		auto PackageNameStr = GetPackageName();
		auto AssetNameStr = GetAssetName();

		if (!AssetNameStr.empty())
			return PackageNameStr + '.' + AssetNameStr;

		return PackageNameStr;
	}

private:

	FName PackageName;
	FName AssetName;
};