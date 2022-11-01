#pragma once

#include "../Property.h"
#include "../PropertyValue.h"

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