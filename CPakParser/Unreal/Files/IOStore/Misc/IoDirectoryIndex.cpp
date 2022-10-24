#include "IoDirectoryIndex.h"

#include "Serialization/Archives.h"

FArchive& operator<<(FArchive& Ar, FIoDirectoryIndexResource& DirectoryIndex)
{
	Ar << DirectoryIndex.MountPoint;
	Ar.BulkSerializeArray(DirectoryIndex.DirectoryEntries);
	Ar.BulkSerializeArray(DirectoryIndex.FileEntries);
	Ar << DirectoryIndex.StringTable;

	return Ar;
}