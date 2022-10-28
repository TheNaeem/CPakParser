#pragma once

#include "../Property.h"

class FMapProperty : public FProperty
{
public:

	friend class FPropertyFactory;

private:

	FProperty* KeyType;
	FProperty* ValueType;

public:

	__forceinline FProperty* GetKeyProp()
	{
		return KeyType;
	}

	__forceinline FProperty* GetValueProp()
	{
		return ValueType;
	}
};