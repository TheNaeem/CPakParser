#pragma once

#include "Text.h"
#include "../Hashing/Map.h"

struct FLocalization
{
	TMap<FTextId, std::string> Entries;

	friend FArchive& operator<<(FArchive& Ar, FLocalization& Localization);
};