#pragma once

#include "DiskFile.h"

struct FFileEntryInfo
{
	FFileEntryInfo()
	{
		Entry.TocIndex = NULL;
	};

	FFileEntryInfo(uint32_t InIndex) { Entry.TocIndex = InIndex; }
	FFileEntryInfo(int32_t InIndex) { Entry.PakIndex = InIndex; }

	friend FArchive& operator<<(FArchive& Ar, FFileEntryInfo& EntryInfo);

	__forceinline bool IsValid()
	{
		return Entry.PakIndex;
	}

	__forceinline void SetOwningFile(TWeakPtr<IDiskFile> DiskFile)
	{
		AssociatedFile = DiskFile;
	}

	__forceinline std::string GetDiskFilePath()
	{
		return AssociatedFile.lock()->GetDiskPath();
	}

	__forceinline TSharedPtr<IDiskFile> GetAssociatedFile()
	{
		return AssociatedFile.lock();
	}

	__forceinline uint32_t GetTocIndex()
	{
		return Entry.TocIndex;
	}

	__forceinline int32_t GetPakIndex()
	{
		return Entry.PakIndex;
	}

protected:

	union
	{
		int32_t PakIndex;
		uint32_t TocIndex;
	}Entry;

	TWeakPtr<IDiskFile> AssociatedFile;
};