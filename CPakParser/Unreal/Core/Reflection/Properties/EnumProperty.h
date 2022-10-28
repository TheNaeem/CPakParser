#pragma once

#include "../Property.h"

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
};