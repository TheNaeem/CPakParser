#pragma once

#include "Name.h"

class FMappedName
{
	static constexpr uint32_t InvalidIndex = ~uint32_t(0);
	static constexpr uint32_t IndexBits = 30u;
	static constexpr uint32_t IndexMask = (1u << IndexBits) - 1u;
	static constexpr uint32_t TypeMask = ~IndexMask;
	static constexpr uint32_t TypeShift = IndexBits;

public:

	enum class EType
	{
		Package,
		Container,
		Global
	};

	inline FMappedName() = default;

	static __forceinline FMappedName Create(const uint32_t InIndex, const uint32_t InNumber, EType InType)
	{
		return FMappedName((uint32_t(InType) << TypeShift) | InIndex, InNumber);
	}

	__forceinline bool IsValid() const
	{
		return Index != InvalidIndex && Number != InvalidIndex;
	}

	__forceinline EType GetType() const
	{
		return static_cast<EType>(uint32_t((Index & TypeMask) >> TypeShift));
	}

	__forceinline bool IsGlobal() const
	{
		return ((Index & TypeMask) >> TypeShift) != 0;
	}

	__forceinline uint32_t GetIndex() const
	{
		return Index & IndexMask;
	}

	__forceinline uint32_t GetNumber() const
	{
		return Number;
	}

	__forceinline bool operator!=(FMappedName Other) const
	{
		return Index != Other.Index || Number != Other.Number;
	}

	friend class FArchive& operator<<(FArchive& Ar, FMappedName& MappedName);

private:

	inline FMappedName(const uint32_t InIndex, const uint32_t InNumber)
		: Index(InIndex)
		, Number(InNumber) { }

	uint32_t Index = InvalidIndex;
	uint32_t Number = InvalidIndex;
};