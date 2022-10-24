#include "Text.h"

#include "Serialization/Archives.h"

void FTextKey::Serialize(FArchive& Ar, ELocResVersion& Ver)
{
	if (Ver >= ELocResVersion::Optimized_CityHash64_UTF16)
	{
		Ar << StrHash;
	}
	else if (Ver == ELocResVersion::Optimized_CRC32)
	{
		Ar.SeekCur<uint32_t>();
	}
	
	Ar << Str;
}

FArchive& operator<<(FArchive& Ar, FTextLocalizationResourceString& A)
{
	Ar << A.String;
	Ar.SeekCur<uint32_t>();

	return Ar;
}

size_t hash_value(const FTextId& i)
{
	return phmap::HashState().combine(0, hash_value(i.Key), hash_value(i.Namespace));
}