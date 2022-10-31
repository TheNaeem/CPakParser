#pragma once

#include "../Property.h"
#include "../PropertyValue.h"
#include "Misc/Delegates/ScriptDelegate.h"

struct FDelegateProperty : public FProperty
{
	struct Value : public IPropValue
	{
		FScriptDelegate Delegate;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::DelegateProperty;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			if (Type == EPropertyType::DelegateProperty)
			{
				memcpy(OutBuffer, &Delegate, sizeof(Delegate));
			}
			else if (Type == EPropertyType::StrProperty)
			{
				*((std::string*)OutBuffer) = Delegate.GetFunctionName();
			}
		}
	};

	TUniquePtr<IPropValue> Serialize(FSharedAr Ar) override;
};