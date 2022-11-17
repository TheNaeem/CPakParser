#pragma once

#include "Misc/Hashing/Map.h"

import CPakParser.Core.UObject;
import <string>;

class Mappings // make it a class instead of a namespace so it can be a friend to other classes
{
public:

	static bool RegisterTypesFromUsmap(std::string Path, TMap<std::string, UObjectPtr>& ObjectArray);
};