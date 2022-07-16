#pragma once

struct FIoDirectoryIndexEntry
{
	uint32_t Name = ~uint32_t(0);
	uint32_t FirstChildEntry = ~uint32_t(0);
	uint32_t NextSiblingEntry = ~uint32_t(0);
	uint32_t FirstFileEntry = ~uint32_t(0);

	friend FArchive& operator<<(FArchive& Ar, FIoDirectoryIndexEntry& Entry)
	{
		Ar << Entry.Name;
		Ar << Entry.FirstChildEntry;
		Ar << Entry.NextSiblingEntry;
		Ar << Entry.FirstFileEntry;

		return Ar;
	}
};

struct FIoFileIndexEntry
{
	uint32_t Name = ~uint32_t(0);
	uint32_t NextFileEntry = ~uint32_t(0);
	uint32_t UserData = 0;

	friend FArchive& operator<<(FArchive& Ar, FIoFileIndexEntry& Entry)
	{
		Ar << Entry.Name;
		Ar << Entry.NextFileEntry;
		Ar << Entry.UserData;

		return Ar;
	}
};

struct FIoDirectoryIndexResource
{
	std::string MountPoint;
	std::vector<FIoDirectoryIndexEntry> DirectoryEntries;
	std::vector<FIoFileIndexEntry> FileEntries;
	std::vector<std::string> StringTable;

	friend FArchive& operator<<(FArchive& Ar, FIoDirectoryIndexResource& DirectoryIndex)
	{
		Ar << DirectoryIndex.MountPoint;
		Ar.BulkSerializeArray(DirectoryIndex.DirectoryEntries);
		Ar.BulkSerializeArray(DirectoryIndex.FileEntries);
		Ar << DirectoryIndex.StringTable;

		return Ar;
	}
};