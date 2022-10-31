#pragma once

#include "Core/UObject.h"
#include "Core/Names/Name.h"

class FScriptDelegate
{
	UObjectPtr Object;
	FName FunctionName;

public:

	friend class FArchive& operator<<(FArchive& Ar, FScriptDelegate& Delegate);

	__forceinline std::string GetFunctionName()
	{
		return FunctionName.ToString();
	}

	__forceinline UObjectPtr GetObject()
	{
		return Object;
	}
};