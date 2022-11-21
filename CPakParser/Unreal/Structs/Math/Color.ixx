export module CPakParser.Math.Color;

import CPakParser.Serialization.FArchive;
import <cstdint>;

export struct FColor
{
	FColor() = default;

	constexpr __forceinline FColor(uint8_t InR, uint8_t InG, uint8_t InB, uint8_t InA = 255)
		: B(InB), G(InG), R(InR), A(InA)
	{}

	union { struct { uint8_t B, G, R, A; }; uint32_t AlignmentDummy; };

	uint32_t& DWColor(void) { return *((uint32_t*)this); }
	const uint32_t& DWColor(void) const { return *((uint32_t*)this); }

	friend FArchive& operator<< (FArchive& Ar, FColor& Color)
	{
		return Ar << Color.DWColor();
	}
};