module;

#include "Core/Defines.h"

export module CPakParser.Files.DiskFile;

import <string>;
import <vector>;
import CPakParser.Package;

export class IDiskFile
{
public:

	virtual std::string GetDiskPath() = 0;
	virtual std::vector<uint8_t> ReadEntry(struct FFileEntryInfo& Entry) = 0;
	virtual UPackagePtr CreatePackage(class FArchive& Ar, TSharedPtr<class GContext> Context) = 0;
};