#pragma once

#include "../Property.h"

class FSetProperty : public FProperty
{
public:

	friend class FPropertyFactory;

private:

	FProperty* ElementType;

public:

	__forceinline FProperty* GetElementType()
	{
		return ElementType;
	}
};