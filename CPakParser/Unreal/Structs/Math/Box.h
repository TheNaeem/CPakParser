#pragma once

#include "Vector.h"
#include <cstdint>

struct FBox
{
	FBox() = default;

	FVector Min;
	FVector Max;
	uint8_t IsValid;

	friend class FArchive& operator<<(class FArchive& Ar, FBox& Box);
};