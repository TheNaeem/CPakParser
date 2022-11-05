#pragma once

#include "Core/Class.h"
#include "../Impl/ExportReader.h"

struct FUnversionedSerializer
{
	static void SerializeUnversionedProperties(UStructPtr Struct, FSharedAr Ar, UObjectPtr Object);
};