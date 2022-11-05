#include "TopLevelAssetPath.h"

#include "Serialization/Archives.h"

FTopLevelAssetPath::FTopLevelAssetPath(FName& Name)
{
	auto NameStr = Name.ToString();
	std::string_view View = NameStr;

	if (View.empty() || View == "None")
		return;

	if (View[0] != '/' || View[View.size() - 1] == '\'')
	{
		auto Index = View.find('\'');
		if (Index != std::string_view::npos and View[View.size() - 1] == '\'')
			View = View.substr(Index + 1, View.size() - Index - 2);

		if (View.empty() || View[0] != '/')
			return;
	}

	auto DotIndex = View.find('.');

	if (DotIndex == std::string::npos)
		return;

	auto PackageNameView = View.substr(0, DotIndex);
	auto AssetNameView = View.substr(PackageNameView.size() + 1);

	if (AssetNameView.empty())
	{
		PackageName = FName(PackageNameView);
		AssetName = FName();
		return;
	}

	if (AssetNameView.find('.') != std::string_view::npos or
		AssetNameView.find(':') != std::string_view::npos)
	{
		return;
	}

	PackageName = FName(PackageNameView);
	AssetName = FName(AssetNameView);
}

FArchive& operator<<(FArchive& Ar, FTopLevelAssetPath& Value)
{
	Ar << Value.PackageName;
	Ar << Value.AssetName;

	return Ar;
}