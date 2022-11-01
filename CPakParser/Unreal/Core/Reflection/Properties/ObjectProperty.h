#pragma once

#include "../Property.h"

class FObjectProperty : public FProperty
{
public:

	struct Value : public IPropValue
	{
	public:

		UObjectPtr Object;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::ObjectProperty;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			if (Type == EPropertyType::ObjectProperty)
				memcpy(OutBuffer, &Object, sizeof(Object));
			
		}
	};

	TUniquePtr<class IPropValue> Serialize(FSharedAr Ar) override;
};