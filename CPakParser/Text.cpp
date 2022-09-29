#include "Text.h"

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