#pragma once

#include "Core/Class.h"
#include "../Impl/ExportReader.h"

namespace UnversionedSerializer
{
	void SerializeUnversionedProperties(UStructPtr Struct, TSharedPtr<FExportReader> Ar, uint8_t* Data, UStructPtr DefaultsStruct, uint8_t* DefaultsData);
}