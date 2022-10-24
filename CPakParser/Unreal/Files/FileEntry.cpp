#include "FileEntry.h"

#include "Serialization/Archives.h"

FArchive& operator<<(FArchive& Ar, FFileEntryInfo& Info)
{
	return Ar << Info.Entry.PakIndex;
}