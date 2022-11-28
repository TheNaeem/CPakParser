module;

#include "Core/Defines.h"

export module CPakParser.MathUtil;

export namespace CPP_Math
{
	__forceinline bool IsNearlyZero(float Value, float ErrorTolerance = UE_SMALL_NUMBER)
	{
		return std::abs(Value) <= ErrorTolerance;
	}

	__forceinline bool IsNearlyZero(double Value, double ErrorTolerance = UE_DOUBLE_SMALL_NUMBER)
	{
		return std::abs(Value) <= ErrorTolerance;
	}
}