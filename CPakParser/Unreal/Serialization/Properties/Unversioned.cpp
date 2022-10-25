#include "Unversioned.h"

#include <bitset>
#include <vector>

struct FUnversionedHeader
{
	struct FFragment
	{
		static constexpr uint32_t SkipMax = 127;
		static constexpr uint32_t ValueMax = 127;

		uint8_t SkipNum = 0;
		bool bHasAnyZeroes = false;
		uint8_t ValueNum = 0;
		bool bIsLast = 0;

		static constexpr uint32_t SkipNumMask = 0x007fu;
		static constexpr uint32_t HasZeroMask = 0x0080u;
		static constexpr uint32_t ValueNumShift = 9u;
		static constexpr uint32_t IsLastMask = 0x0100u;

		FFragment() = default;

		__forceinline constexpr FFragment(uint16_t Int)
		{
			SkipNum = static_cast<uint8_t>(Int & SkipNumMask);
			bHasAnyZeroes = (Int & HasZeroMask) != 0;
			ValueNum = static_cast<uint8_t>(Int >> ValueNumShift);
			bIsLast = (Int & IsLastMask) != 0;
		}
	};
	
	friend FArchive& operator<<(FArchive& Ar, FUnversionedHeader& Header)
	{
		uint32_t ZeroMaskNum = 0;
		uint32_t UnmaskedNum = 0;

		while (true)
		{
			uint16_t Packed;
			Ar << Packed;

			FFragment Fragment(Packed);

			Header.Fragments.push_back(Fragment);

			(Fragment.bHasAnyZeroes ? ZeroMaskNum : UnmaskedNum) += Fragment.ValueNum;

			if (Fragment.bIsLast)
				break;
		}

		if (ZeroMaskNum)
		{
			std::vector<bool>& ZeroMask = Header.ZeroMask;

			ZeroMask.resize(ZeroMaskNum, 0);

			auto Data = (uint8_t*)ZeroMask[0]._Getptr(); // TODO: change this once phmap is fixed so we can use std::latest

			if (ZeroMaskNum <= 8)
			{
				Ar.Serialize(Data, sizeof(uint8_t));
			}
			else if (ZeroMaskNum <= 16)
			{
				Ar.Serialize(Data, sizeof(uint16_t));
			}
			else
			{
				for (uint32_t Idx = 0, Num = (ZeroMaskNum + 32 - 1) / 32; Idx < Num; ++Idx)
				{
					Ar << Data[Idx];
				}
			}

			Header.bHasNonZeroValues = UnmaskedNum > 0 || std::find(ZeroMask.begin(), ZeroMask.end(), false) == ZeroMask.end();
		}
		else
		{
			Header.bHasNonZeroValues = UnmaskedNum > 0;
		}

		return Ar;
	}

	std::vector<FFragment> Fragments; // TODO: what do we need?
	bool bHasNonZeroValues = false;
	std::vector<bool> ZeroMask;

	__forceinline bool HasValues() const
	{
		return bHasNonZeroValues | (ZeroMask.size() > 0);
	}

	__forceinline bool HasNonZeroValues() const
	{
		return bHasNonZeroValues;
	}
};

void UnversionedSerializer::SerializeUnversionedProperties(UStructPtr Struct, TSharedPtr<FExportReader> Ar, uint8_t* Data, UStructPtr DefaultsStruct, uint8_t* DefaultsData)
{
	FUnversionedHeader Header;
	*Ar << Header;

	if (!Header.HasValues() || !Header.HasNonZeroValues())
		return;

	uint32_t ZeroMaskIndex = 0;

	for (auto& Fragment : Header.Fragments)
	{
		ZeroMaskIndex += Fragment.bHasAnyZeroes;

		if (Fragment.bHasAnyZeroes && Header.ZeroMask[ZeroMaskIndex])
		{
			continue;
		}


	}
}