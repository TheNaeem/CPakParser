#pragma once

#include "../Property.h"
#include "../PropertyValue.h"
#include <vector>

class FArrayProperty : public FProperty
{
public:

	friend class FPropertyFactory;

	struct Value : public IPropValue
	{
	public:

		std::vector<TUniquePtr<class IPropValue>> Array;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::ArrayProperty;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			// TODO:
		}
	};

	TUniquePtr<class IPropValue> Serialize(FSharedAr Ar) override;

private:

	FProperty* ElementType;

public:

	__forceinline FProperty* GetElementType()
	{
		return ElementType;
	}
};