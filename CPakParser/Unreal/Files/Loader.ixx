module;

#include "Core/Defines.h"

export module CPakParser.Files.Loader;

import CPakParser.Serialization.FArchive;
import CPakParser.Files.FileEntry;
import CPakParser.Package.FileSummary;

export class FLoader // TODO: make this completely different and obselete
{
private:

	FSharedAr Reader;
	class UPackage& Package;
	bool bHasSerializedPackageFileSummary;

	FLoader(class UPackage& InPackage);

	void SerializePackageSummary();

public:

	~FLoader();

	__forceinline bool IsValid();
	static TSharedPtr<FLoader> FromPackage(class UPackage& Package);
	static FSharedAr CreateFileReader(FFileEntryInfo Entry, bool bMemoryPreload = true);
	void LoadAllObjects();

	FPackageFileSummary	Summary;
};