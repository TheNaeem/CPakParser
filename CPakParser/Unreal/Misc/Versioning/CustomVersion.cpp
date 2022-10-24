#include "CustomVersion.h"

#include "Serialization/Archives.h"

bool FCustomVersion::operator==(FGuid InKey) const
{
	return Key == InKey;
}

bool FCustomVersion::operator!=(FGuid InKey) const
{
	return Key != InKey;
}

void FCustomVersionContainer::Serialize(FArchive& Ar, ECustomVersionSerializationFormat Format)
{
	switch (Format) // implement the deprecated formats if needed but i doubt it ever will
	{
	case ECustomVersionSerializationFormat::Optimized:
	{
		Ar << Versions;
	}
	break;
	}
}

FArchive& operator<<(FArchive& Ar, FCustomVersion& Version)
{
	/*
	Ar << Version.Key;
	Ar << Version.Version;
	*/

	Ar.Serialize(&Version.Key, sizeof(Version.Key) + sizeof(Version.Version));

	return Ar;
}