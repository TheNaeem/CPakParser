#include "Vector.h"

#include "Serialization/Archives.h"

FArchive& operator<<(FArchive& Ar, FVector& Value)
{
	if (Ar.UEVer() >= EUnrealEngineObjectUE5Version::LARGE_WORLD_COORDINATES)
	{
		Ar.Serialize(&Value, sizeof(FVector));
	}
	else
	{
		float X, Y, Z;
		Ar << X << Y << Z;

	}

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FVector2D& V)
{
	if (Ar.UEVer() >= EUnrealEngineObjectUE5Version::LARGE_WORLD_COORDINATES)
	{
		Ar.Serialize(&V, sizeof(FVector2D));
	}
	else
	{
		float X, Y;
		Ar << X << Y;
		V = FVector2D(X, Y);
	}

	return Ar;
}