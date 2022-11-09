#include "Box.h"

#include "Serialization/Archives.h"

FArchive& operator<<(FArchive& Ar, FBox& Box)
{
	return Ar << Box.Min << Box.Max << Box.IsValid;
}

