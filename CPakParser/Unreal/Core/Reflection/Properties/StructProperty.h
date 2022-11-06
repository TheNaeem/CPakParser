#pragma once

#include "../Property.h"
#include "../PropertyValue.h"

#include "Core/Class.h"

class FStructProperty : public FProperty
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

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override;

private:

	UStructPtr Struct;
};