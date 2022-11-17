export module CPakParser.Math.FBox;

import CPakParser.Math.FVector;
import CPakParser.Serialization.FArchive;

struct FBox
{
	FBox() = default;

	FVector Min;
	FVector Max;
	uint8_t IsValid;

	friend FArchive& operator<<(FArchive& Ar, FBox& Box)
	{
		return Ar << Box.Min << Box.Max << Box.IsValid;
	}
};