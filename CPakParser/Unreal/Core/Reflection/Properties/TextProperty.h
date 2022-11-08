#pragma once

#include "../PropertyValue.h"
#include "Misc/Localization/Text.h"

class FTextProperty : public FProperty
{
public:

	class Value : public IPropValue
	{
	public:

		FText Text;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return false; // TODO: struct stuff
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			// TODO:
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override;
};