#pragma once

#include "FileEntry.h"
#include "Core/Defines.h"
#include "Packaging/PackageSummary.h"

class UPackage;

class FLoader // TODO: make this completely different and obselete
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