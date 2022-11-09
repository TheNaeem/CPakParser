#pragma once

#include "../Property.h"
#include "../PropertyValue.h"
#include "Misc/Delegates/MulticastScriptDelegate.h"

struct FMulticastDelegateProperty : public FProperty
{
	struct Value : public IPropValue
	{
		FMulticastScriptDelegate Delegate;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::MulticastDelegateProperty;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			if (Type == EPropertyType::MulticastDelegateProperty)
			{
				*((FMulticastScriptDelegate*)OutBuffer) = Delegate;
			}
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override;
};