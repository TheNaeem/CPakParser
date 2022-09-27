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

	void CreateLoader(bool bMemoryPreload);
	void SerializePackageSummary();

public:

	~FLoader();

	__forceinline bool IsValid();
	static std::shared_ptr<FLoader> FromPackage(UPackage& Package);
	void LoadAllObjects();

	FPackageFileSummary	Summary;
};