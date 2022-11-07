#pragma once

#include "Core/Names/Name.h"

class FGameplayTag
{
public:

	friend void operator<<(class FArchive& Ar, FGameplayTag& GameplayTag);

	__forceinline std::string ToString() const
	{
		return TagName.ToString();
	}

	__forceinline FName GetName() const
	{
		return TagName;
	}

private:

	FName TagName;
};