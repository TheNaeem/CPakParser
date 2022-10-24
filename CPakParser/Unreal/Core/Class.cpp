#include "Class.h"

#include "Serialization/Impl/ExportReader.h"

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

	FFragment(uint16_t Int)
	{
		SkipNum = static_cast<uint8_t>(Int & SkipNumMask);
		bHasAnyZeroes = (Int & HasZeroMask) != 0;
		ValueNum = static_cast<uint8_t>(Int >> ValueNumShift);
		bIsLast = (Int & IsLastMask) != 0;
	}

	friend FArchive& operator<<(FArchive& Ar, FFragment& Fragment)
	{
		while (true)
		{
			uint16_t Packed;
			Ar << Packed;

			FFragment Fragment(Packed);

			if (Fragment.bIsLast)
				break;
		}

		return Ar;
	}
};

void SerializeUnversionedProperties(UStructPtr Struct, TSharedPtr<FExportReader> Ar, uint8_t* Data, UStructPtr DefaultsStruct, uint8_t* DefaultsData)
{
	FFragment Fragment;
	*Ar << Fragment;
}

UStructPtr UStruct::GetSuper()
{
	return Super;
}

void UStruct::SetSuper(UStructPtr Val)
{
	Super = Val;
}

void UStruct::SerializeScriptProperties(TSharedPtr<FExportReader> Ar, uint8_t* Data, UStructPtr DefaultsStruct, uint8_t* Defaults)
{
	if (Ar->UseUnversionedPropertySerialization())
	{
		SerializeUnversionedProperties(This<UStruct>(), Ar, Data, DefaultsStruct, Defaults);
		return;
	}

	// TODO: SerializeVersionedTaggedProperties
}