#pragma once

#include <cstdint>

class FDateTime
{
public:

	FDateTime() = default;

	FDateTime(int64_t InTicks) : Ticks(InTicks)
	{
	}

	friend class FArchive& operator<<(FArchive& Ar, FDateTime& DateTime);

	__forceinline int64_t GetTicks()
	{
		return Ticks;
	}

private:
	int64_t Ticks;
};