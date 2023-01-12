module;

#include "Core/Defines.h"

export module CPakParser.Reflection.SetProperty;

import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;

export class FSetProperty : public FProperty // TODO ASAP: set properties support
{
public:

	friend class FPropertyFactory;

private:

	FProperty* ElementType;

public:

	class Value : public IPropValue
	{
	public:

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::SetProperty; // TODO: struct stuff
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
	{
		auto Ret = std::make_unique<Value>();

		int32_t NumElementsToRemove = 0;
		Ar << NumElementsToRemove;

		for (; NumElementsToRemove; --NumElementsToRemove)
		{
			ElementType->Serialize(Ar); 
		}

		int32_t Num = 0;
		Ar << Num;

		for (; Num; --Num)
		{
			ElementType->Serialize(Ar); // TODO: store this
		}

		return std::move(Ret);
	}

	__forceinline FProperty* GetElementType()
	{
		return ElementType;
	}
};