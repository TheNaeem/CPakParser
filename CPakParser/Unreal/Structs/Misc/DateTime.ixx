export module CPakParser.Structs.DateTime;

import CPakParser.Serialization.FArchive;

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