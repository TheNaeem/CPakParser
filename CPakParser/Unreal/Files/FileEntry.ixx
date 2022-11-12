export module FileEntry;

export import DiskFile;
import FArchiveBase;
import <string>;

export struct FFileEntryInfo
{
	FFileEntryInfo()
	{
		Entry.TocIndex = 0;
	};

	FFileEntryInfo(uint32_t InIndex) { Entry.TocIndex = InIndex; }
	FFileEntryInfo(int32_t InIndex) { Entry.PakIndex = InIndex; }

	friend FArchive& operator<<(FArchive& Ar, FFileEntryInfo& Info)
	{
		return Ar << Info.Entry.PakIndex;
	}

	__forceinline bool IsValid()
	{
		return Entry.PakIndex;
	}

	__forceinline void SetOwningFile(IDiskFile* DiskFile)
	{
		AssociatedFile = DiskFile;
	}

	__forceinline std::string GetDiskFilePath()
	{
		return AssociatedFile->GetDiskPath();
	}

	__forceinline IDiskFile* GetAssociatedFile()
	{
		return AssociatedFile;
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

	IDiskFile* AssociatedFile; // TODO: make this an int index
};