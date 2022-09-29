#pragma once

#include "Archives.h"
#include "Package.h"
#include "PackageSummary.h"

class UPackage;

class FLoader
{
private:

	FArchive* Reader;
	UPackage& Package;
	bool bHasSerializedPackageFileSummary;

	FLoader(UPackage& InPackage);

	void SerializePackageSummary();

public:

	~FLoader();

	__forceinline bool IsValid();
	static std::shared_ptr<FLoader> FromPackage(UPackage& Package);
	static FArchive* CreateFileReader(FGameFilePath Path, bool bMemoryPreload = true);
	void LoadAllObjects();

	FPackageFileSummary	Summary;
};