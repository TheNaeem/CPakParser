export module DateTime;

import FArchiveBase;

class FDateTime
{
public:

	FDateTime() = default;

	FDateTime(__int64 InTicks) : Ticks(InTicks)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FDateTime& DateTime)
	{
		return Ar << DateTime.Ticks;
	}

	__int64 GetTicks()
	{
		return Ticks;
	}

private:
	__int64 Ticks;
};