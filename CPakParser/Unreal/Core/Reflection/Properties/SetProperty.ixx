module;

#include "Core/Defines.h"

export module CPakParser.Reflection.SetProperty;

import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;

export class FSetProperty : public FProperty
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