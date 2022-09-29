#pragma once

#include "Text.h"

constexpr FGuid LOCRES_MAGIC = FGuid(0x7574140E, 0xFC034A67, 0x9D90154A, 0x1B7F37C3);
constexpr FGuid LOCMETA_MAGIC = FGuid(0xA14CEE4F, 0x83554868, 0xBD464C6C, 0x7C50DA70);

struct FLocalization
{
	phmap::flat_hash_map<FTextId, std::string> Entries;

	friend FArchive& operator<<(FArchive& Ar, FLocalization& Localization);
};