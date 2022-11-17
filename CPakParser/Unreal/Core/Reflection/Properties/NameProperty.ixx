module;

#include "Core/Defines.h"

export module CPakParser.Reflection.NameProperty;

import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;
import CPakParser.Core.FName;

export class FNameProperty : public FProperty
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
				((std::string*)OutBuffer)->assign(Name.GetString());
			}
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
	{
		auto Ret = std::make_unique<Value>();
		Ar << Ret->Name;

		return std::move(Ret);
	}
};