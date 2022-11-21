export module CPakParser.Math.FBox;

export import CPakParser.Math.FVector;
import CPakParser.Serialization.FArchive;

import <cstdint>;

export struct FBox
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

export struct FBox2D
{
	FBox2D() = default;

	FVector2D Min;
	FVector2D Max;
	bool bIsValid;

	friend FArchive& operator<<(FArchive& Ar, FBox2D& Box)
	{
		return Ar << Box.Min << Box.Max << Box.bIsValid;
	}
};