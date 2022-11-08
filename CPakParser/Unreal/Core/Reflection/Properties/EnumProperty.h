#pragma once

#include "../Property.h"
#include "../PropertyValue.h"

#include <vector>

struct FReflectedEnum // traits that can be shared across multiple instances of the same enum property
{
	std::vector<std::string> Enum;
	std::string EnumName;
};

class FEnumProperty : public FProperty
{
public: 

	friend class FPropertyFactory;

	struct Value : public IPropValue
	{
		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::EnumProperty;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
		}
	};

private:

	TSharedPtr<FReflectedEnum> Enum;
	FProperty* UnderlyingProp;

public:

	__forceinline FProperty* GetUnderlying()
	{
		return UnderlyingProp;
	}

	__forceinline std::vector<std::string> GetValues()
	{
		return Enum->Enum;
	}

	__forceinline std::string GetEnumName()
	{
		return Enum->EnumName;
	}

	TUniquePtr<IPropValue> Serialize(class FArchive& Ar) override;
};