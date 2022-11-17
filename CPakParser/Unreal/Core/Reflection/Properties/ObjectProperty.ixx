module;

#include "Core/Defines.h"

export module CPakParser.Reflection.ObjectProperty;

import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;
import CPakParser.Core.UObject;

export class FObjectProperty : public FProperty
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
				*((UObjectPtr*)OutBuffer) = Object;
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
	{
		auto Ret = std::make_unique<Value>();
		Ar << Ret->Object;

		return std::move(Ret);
	}
};