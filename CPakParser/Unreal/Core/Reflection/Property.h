#pragma once

#include "../Defines.h"

#include <string>

/*
* I made the decision to use raw pointers with properties.
* Mainly to avoid the overhead of using a shared pointer.
* Also, properties should only live as long as their parent class, which handles freeing them.
*/

class FProperty
{
public:

	friend class FPropertyFactory;

private:

	std::string Name;
	uint16_t Index;
	uint8_t ArrayDim;
	FProperty* Next;

public:

	__forceinline std::string GetName()
	{
		return Name;
	}

	__forceinline uint16_t GetIndex()
	{
		return Index;
	}

	__forceinline uint8_t GetArrayDim()
	{
		return ArrayDim;
	}

	__forceinline FProperty* GetNext()
	{
		return Next;
	}
};