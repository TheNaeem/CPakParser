module;

#include "Core/Defines.h"

export module CPakParser.Math.FrameNumber;

import <type_traits>;
import <algorithm>;
import CPakParser.Serialization.FArchive;

export struct FFrameNumber
{
	constexpr FFrameNumber()
		: Value(0)
	{}

	constexpr explicit FFrameNumber(int32_t InValue)
		: Value(InValue)
	{}

	friend FArchive& operator<<(FArchive& Ar, FFrameNumber& FrameNumber)
	{
		return Ar << FrameNumber.Value;
	}

	FFrameNumber& operator+=(FFrameNumber RHS) { Value += RHS.Value; return *this; }
	FFrameNumber& operator-=(FFrameNumber RHS) { Value -= RHS.Value; return *this; }
	FFrameNumber& operator%=(FFrameNumber RHS) { Value %= RHS.Value; return *this; }

	FFrameNumber& operator++() { ++Value; return *this; }
	FFrameNumber& operator--() { --Value; return *this; }

	FFrameNumber operator++(int32_t) { FFrameNumber Ret = *this; ++Value; return Ret; }
	FFrameNumber operator--(int32_t) { FFrameNumber Ret = *this; --Value; return Ret; }

	friend bool operator==(FFrameNumber A, FFrameNumber B) { return A.Value == B.Value; }
	friend bool operator!=(FFrameNumber A, FFrameNumber B) { return A.Value != B.Value; }

	friend bool operator< (FFrameNumber A, FFrameNumber B) { return A.Value < B.Value; }
	friend bool operator> (FFrameNumber A, FFrameNumber B) { return A.Value > B.Value; }
	friend bool operator<=(FFrameNumber A, FFrameNumber B) { return A.Value <= B.Value; }
	friend bool operator>=(FFrameNumber A, FFrameNumber B) { return A.Value >= B.Value; }

	friend FFrameNumber operator+(FFrameNumber A, FFrameNumber B) { return FFrameNumber(A.Value + B.Value); }
	friend FFrameNumber operator-(FFrameNumber A, FFrameNumber B) { return FFrameNumber(A.Value - B.Value); }
	friend FFrameNumber operator%(FFrameNumber A, FFrameNumber B) { return FFrameNumber(A.Value % B.Value); }

	friend FFrameNumber operator-(FFrameNumber A) { return FFrameNumber(-A.Value); }

	friend FFrameNumber operator*(FFrameNumber A, float Scalar) { return FFrameNumber(static_cast<int32_t>(std::clamp(floor(double(A.Value) * Scalar), (double)MIN_int32, (double)MAX_int32))); }
	friend FFrameNumber operator/(FFrameNumber A, float Scalar) { return FFrameNumber(static_cast<int32_t>(std::clamp(floor(double(A.Value) / Scalar), (double)MIN_int32, (double)MAX_int32))); }

	friend inline uint32_t hash_value(FFrameNumber A)
	{
		return A.Value;
	}

	int32_t Value;
};