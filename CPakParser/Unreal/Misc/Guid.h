#pragma once

#include <string>

struct FGuid
{
	int32_t A, B, C, D;

	constexpr FGuid(int32_t InA, int32_t InB, int32_t InC, int32_t InD)
		: A(InA), B(InB), C(InC), D(InD)
	{ }

	FGuid() :
		A(0),
		B(0),
		C(0),
		D(0)
	{
	}

	FGuid(std::string GuidStr);

	friend class FArchive& operator<<(class FArchive& Ar, FGuid& Value);

	bool operator==(FGuid Other) const
	{
		return Other.A == A && Other.B == B && Other.C == C && Other.D == D;
	}

	bool operator!=(FGuid Other) const
	{
		return !(*this == Other);
	}

	bool IsValid() const
	{
		return ((A | B | C | D) != 0);
	}

	void Invalidate()
	{
		A = B = C = D = 0;
	}

	friend size_t hash_value(const FGuid& Guid);
};