#pragma once

#include "Property.h"

class IPropValue
{
public:

	virtual bool IsAcceptableType(EPropertyType Type) = 0;
	virtual void PlaceValue(EPropertyType Type, void* OutBuffer) = 0;
};