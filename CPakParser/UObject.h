#pragma once

#include "CoreTypes.h"

class UObjectPtr
{
	TSharedPtr<UObject> Val;

public:

	UObjectPtr() : Val(nullptr)
	{
	}

	UObjectPtr operator=(TSharedPtr<UObject> Other)
	{
		Val = Other;
		return *this;
	}

	UObjectPtr(TSharedPtr<UObject> InObject) : Val(InObject)
	{
	}

	__forceinline UObject* operator->()
	{
		if (!Val)
		{
			// TODO: load it
		}

		return Val.get();
	}

	__forceinline UObject* Get()
	{
		return Val.get();
	}

	__forceinline operator bool() const
	{
		return Val.operator bool();
	}
};

class UObject
{
public:

	friend class UZenPackage;

protected:

	UObjectPtr Class;
	UObjectPtr Outer;
	UObjectPtr Super;
	std::string Name;

public:

	__forceinline std::string GetName()
	{
		return Name;
	}

	__forceinline UObjectPtr GetClass()
	{
		return Class;
	}

	__forceinline UObjectPtr GetSuper()
	{
		return Super;
	}
};