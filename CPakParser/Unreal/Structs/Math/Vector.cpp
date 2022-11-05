#include "Vector.h"

#include "Serialization/Archives.h"

FArchive& operator<<(FArchive& Ar, FVector& Value)
{
	Ar.Serialize(&Value, sizeof(FVector));
	return Ar;
}