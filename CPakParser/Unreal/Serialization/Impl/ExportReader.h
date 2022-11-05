#pragma once

#include "MemoryReader.h"
#include "Core/UObject.h"

class FExportReader : public FMemoryReader // TODO: is this necessary? should we just use FMemoryReader?
{
public:

	FExportReader(uint8_t* InBytes, size_t Size, bool bFreeBuffer = false)
		: FMemoryReader(InBytes, Size, bFreeBuffer)
	{
	}

	virtual void Preload(UObjectPtr Obj);
};