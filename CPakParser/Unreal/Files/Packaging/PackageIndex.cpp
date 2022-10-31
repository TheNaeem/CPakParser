#include "PackageIndex.h"

#include "Serialization/Archives.h"

FArchive& operator<<(FArchive& Ar, FPackageIndex& Value)
{
	return Ar << Value.Index;
}