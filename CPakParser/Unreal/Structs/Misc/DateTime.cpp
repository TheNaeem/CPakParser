#include "DateTime.h"

#include "Serialization/Archives.h"

FArchive& operator<<(FArchive& Ar, FDateTime& DateTime)
{
	return Ar << DateTime.Ticks;
}