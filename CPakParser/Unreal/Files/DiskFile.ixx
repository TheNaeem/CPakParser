module;

#include "Core/Defines.h"

export module DiskFile;

import <string>;

export class IDiskFile
{
public:

	virtual std::string GetDiskPath() = 0;
	virtual TSharedPtr<class FArchive> CreateEntryArchive(struct FFileEntryInfo EntryInfo) = 0;
	virtual void DoWork(TSharedPtr<class FArchive> Ar, TSharedPtr<class GContext> Context) = 0;
};