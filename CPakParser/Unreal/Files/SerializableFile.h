#pragma once

#include "Core/Defines.h"

struct ISerializableFile
{
	virtual void Serialize(FSharedAr Ar) = 0;
};