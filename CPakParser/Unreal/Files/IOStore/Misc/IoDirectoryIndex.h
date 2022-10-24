#pragma once

#include <string>
#include <vector>

struct FIoDirectoryIndexEntry
{
	uint32_t Name = ~uint32_t(0);
	uint32_t FirstChildEntry = ~uint32_t(0);
	uint32_t NextSiblingEntry = ~uint32_t(0);
	uint32_t FirstFileEntry = ~uint32_t(0);
};

struct FIoFileIndexEntry
{
	uint32_t Name = ~uint32_t(0);
	uint32_t NextFileEntry = ~uint32_t(0);
	uint32_t UserData = 0;
};

struct FIoDirectoryIndexResource
{
	std::string MountPoint;
	std::vector<FIoDirectoryIndexEntry> DirectoryEntries;
	std::vector<FIoFileIndexEntry> FileEntries;
	std::vector<std::string> StringTable;

	friend class FArchive& operator<<(FArchive& Ar, FIoDirectoryIndexResource& DirectoryIndex);
};