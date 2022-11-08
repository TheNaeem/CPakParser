#pragma once

#include "../PropertyValue.h"

class FBoolProperty : public FProperty
{
public:

	struct Value : public IPropValue
	{
		bool Val = false;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::BoolProperty;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			memcpy(OutBuffer, &Val, sizeof(Val));
		}
	};

	TUniquePtr<IPropValue> Serialize(class FArchive& Ar) override;
};