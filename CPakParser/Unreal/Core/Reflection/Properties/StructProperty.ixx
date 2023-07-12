module;

#include "Core/Defines.h"

export module CPakParser.Reflection.StructProperty;

import CPakParser.Reflection.FProperty;
import CPakParser.Core.UObject;

export class FStructProperty : public FProperty
{
public:

	friend class FPropertyFactory;

	struct Value : public IPropValue
	{
	public:

		UObjectPtr StructObject;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return false; // TODO: struct stuff
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			// TODO:
		}
	};

	template <typename StructType>
	struct NativeValue : public IPropValue
	{
	public:

		StructType Value;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return false; // TODO: struct stuff
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			// TODO:
		}
	};

	TUniquePtr<IPropValue> Serialize(class FArchive& Ar) override
	{
		return Struct->SerializeItem(Ar);
	}

private:

	UStructPtr Struct;
};