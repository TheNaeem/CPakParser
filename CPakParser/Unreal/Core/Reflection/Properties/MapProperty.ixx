module;

#include "Core/Defines.h"

export module CPakParser.Reflection.MapProperty;

import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;

export class FMapProperty : public FProperty
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