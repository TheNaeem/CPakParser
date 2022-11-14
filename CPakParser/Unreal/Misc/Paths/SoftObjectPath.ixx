export module SoftObjectPath;

import TopLevelAssetPath;
import FArchiveBase;

export class FSoftObjectPath
{
public:

	FSoftObjectPath()
	{
		Reset();
	}

	friend class FArchive& operator<<(class FArchive& Ar, FSoftObjectPath& Value)
	{
		if (Ar.UEVer() < EUnrealEngineObjectUE5Version::FSOFTOBJECTPATH_REMOVE_ASSET_PATH_FNAMES)
		{
			FName AssetPathName;
			Ar << AssetPathName;

			Value.AssetPath = AssetPathName;
			Ar << Value.SubPathString;
		}
		else
		{
			Ar << Value.AssetPath;
			Ar << Value.SubPathString;
		}

		return Ar;
	}

	__forceinline std::string GetAssetPathString() { return AssetPath.ToString(); }
	__forceinline FTopLevelAssetPath GetAssetPath() { return AssetPath; }
	__forceinline std::string GetSubPath() { return SubPathString; }

	void Reset()
	{

		SubPathString = {};
	}

private:

	FTopLevelAssetPath AssetPath;
	std::string SubPathString;
};