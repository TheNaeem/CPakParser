#pragma once

#include "TopLevelAssetPath.h"

class FSoftObjectPath
{
public:
	 
	FSoftObjectPath()
	{
		Reset();
	}

	friend class FArchive& operator<<(class FArchive& Ar, FSoftObjectPath& Value);

	__forceinline std::string GetAssetPathString() { return AssetPath.ToString(); }
	__forceinline FTopLevelAssetPath GetAssetPath() { return AssetPath; }
	__forceinline std::string GetSubPath() { return SubPathString; }

	void SetPath(std::string_view InPath);

	void Reset()
	{

		SubPathString = {};
	}

private:

	FTopLevelAssetPath AssetPath;
	std::string SubPathString;
};