#pragma once

#include "Archives.h"
#include "Package.h"
#include "PackageSummary.h"

class UPackage;

class FLoader // TODO: make this an archive that wraps around the Reader
{
private:

	FSharedAr Reader;
	UPackage& Package;
	bool bHasSerializedPackageFileSummary;

	FLoader(UPackage& InPackage);

	void SerializePackageSummary();

public:

	~FLoader();

	__forceinline bool IsValid();
	static TSharedPtr<FLoader> FromPackage(UPackage& Package);
	static FSharedAr CreateFileReader(FFileEntryInfo Entry, bool bMemoryPreload = true);
	void LoadAllObjects();

	FPackageFileSummary	Summary;
};