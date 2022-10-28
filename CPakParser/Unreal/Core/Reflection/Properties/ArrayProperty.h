#pragma once

#include "../Property.h"

class FArrayProperty : public FProperty
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