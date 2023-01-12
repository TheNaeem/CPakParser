module;

#include "Core/Defines.h"

export module CPakParser.Reflection.MapProperty;

import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;

export class FMapProperty : public FProperty // TODO ASAP:
{
public:

	friend class FPropertyFactory;

private:

	FProperty* KeyType;
	FProperty* ValueType;

public:

	class Value : public IPropValue
	{
	public:

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::MapProperty; // TODO: struct stuff
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{

		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
	{
		auto Ret = std::make_unique<Value>();

		int32_t NumKeysToRemove = 0;
		Ar << NumKeysToRemove;

		for (; NumKeysToRemove; --NumKeysToRemove)
		{
			KeyType->Serialize(Ar);
		}

		int32_t NumEntries = 0;
		Ar << NumEntries;

		for (; NumEntries; --NumEntries)
		{
			KeyType->Serialize(Ar);
			ValueType->Serialize(Ar); // TODO: store these
		}

		return std::move(Ret);
	}

	__forceinline FProperty* GetKeyProp()
	{
		return KeyType;
	}

	__forceinline FProperty* GetValueProp()
	{
		return ValueType;
	}
};