module;

#include "Core/Defines.h"

export module CPakParser.Files.Loader;

import CPakParser.Serialization.FArchive;
import CPakParser.Files.FileEntry;
import CPakParser.Package.FileSummary;

class UPackage;

export class FLoader // TODO: make this completely different and obselete
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