#pragma once

#include "../PropertyValue.h"
#include "Core/Names/Name.h"

class FNameProperty : public FProperty
{
public:

	class Value : public IPropValue
	{
	public:

		FName Name;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::NameProperty or Type == EPropertyType::StrProperty;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			if (Type == EPropertyType::NameProperty)
			{
				*((FName*)OutBuffer) = Name;
			}
			else if (Type == EPropertyType::StrProperty)
			{
				((std::string*)OutBuffer)->assign(Name.Val);
			}
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override;
};