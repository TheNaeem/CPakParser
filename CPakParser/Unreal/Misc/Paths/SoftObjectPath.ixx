export module CPakParser.Paths.SoftObjectPath;

import CPakParser.Paths.TopLevelAssetPath;
import CPakParser.Serialization.FArchive;

export class FSoftObjectPath // TODO: context weak ptr
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

	__forceinline void operator=(FSoftObjectPath& Other)
	{
		AssetPath = Other.AssetPath;
		SubPathString = Other.SubPathString;
	}

	void Reset()
	{
		SubPathString = {};
	}

private:

	FTopLevelAssetPath AssetPath;
	std::string SubPathString;
};