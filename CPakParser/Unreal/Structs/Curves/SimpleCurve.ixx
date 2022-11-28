export module CPakParser.Curves.SimpleCurve;

import CPakParser.Serialization.FArchive;

export struct FSimpleCurveKey
{
	float Time;
	float Value;

	FSimpleCurveKey()
		: Time(0.f)
		, Value(0.f)
	{ }

	FSimpleCurveKey(float InTime, float InValue)
		: Time(InTime)
		, Value(InValue)
	{ }

	friend FArchive& operator<<(FArchive& Ar, FSimpleCurveKey& P)
	{
		Ar << P.Time;
		return Ar << P.Value;
	}
};