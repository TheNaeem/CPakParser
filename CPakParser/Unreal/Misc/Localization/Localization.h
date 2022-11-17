#pragma once

#include "Core/Defines.h"
#include "Misc/Hashing/Map.h"

import CPakParser.Localization.Text;
import CPakParser.Files.SerializableFile;
import <vector>;

export struct FLocalization : public ISerializableFile
{
	TMap<FTextId, std::string> Entries;

	void Serialize(FArchive& Ar) override;
};