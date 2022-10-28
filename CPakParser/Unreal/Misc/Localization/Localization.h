#pragma once

#include "Text.h"
#include "Files/SerializableFile.h"
#include "../Hashing/Map.h"

struct FLocalization : ISerializableFile
{
	TMap<FTextId, std::string> Entries;

	void Serialize(FSharedAr Ar) override;
};