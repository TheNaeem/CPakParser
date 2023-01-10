module;

#include "Core/Defines.h"

export module CPakParser.Reflection.StrProperty;

import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;
import <string>;

export class FStrProperty : public FProperty
{
public:

	class Value : public IPropValue
	{
	public:

		std::string Str;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::StrProperty; // TODO: struct stuff
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			if (Type == EPropertyType::StrProperty)
				*(std::string*)OutBuffer = Str;
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
	{
		auto Ret = std::make_unique<Value>();
		Ar << Ret->Str;

		return std::move(Ret);
	}
};