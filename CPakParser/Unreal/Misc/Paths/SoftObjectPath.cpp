#include "SoftObjectPath.h"

#include "Serialization/Archives.h"
#include <set>

FArchive& operator<<(FArchive& Ar, FSoftObjectPath& Value)
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