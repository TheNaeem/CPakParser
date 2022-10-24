#pragma once

#include "MemoryReader.h"
#include "Core/UObject.h"

class FExportReader : public FMemoryReader
{
	UObjectPtr Archetype;

public:

	FExportReader(uint8_t* InBytes, size_t Size, bool bFreeBuffer = false)
		: FMemoryReader(InBytes, Size, bFreeBuffer)
	{
	}

	__forceinline void SetArchetype(UObjectPtr Val)
	{
		Archetype = Val;
	}

	__forceinline UObjectPtr GetArchetype()
	{
		return Archetype;
	}

	virtual void Preload(UObjectPtr Obj);
};