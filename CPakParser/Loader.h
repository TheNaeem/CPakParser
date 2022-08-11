#pragma once

#include "Archives.h"
#include "Package.h"

class UPackage;

class FLoader
{
private:

	FArchive Reader;
	UPackage& Package;

	FLoader(UPackage& InPackage);

	void CreateLoader();

public:

	__forceinline bool IsValid();
	static std::shared_ptr<FLoader> FromPackage(UPackage& Package);
	void LoadAllObjects();
};