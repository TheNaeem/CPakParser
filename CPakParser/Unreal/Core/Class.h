#pragma once

#include "UObject.h"
#include "Reflection/Property.h"
#include <vector>

class UStruct : public UObject
{
public:

	friend class UObject;
	friend class Mappings;

	~UStruct();

private:

	UStructPtr Super;
	FProperty* PropertyLink = nullptr;

public:

	void SetSuper(UStructPtr Val);
	UStructPtr GetSuper();

	__forceinline FProperty* GetPropertyLink()
	{
		return PropertyLink;
	}

	void SerializeScriptProperties(FArchive& Ar, UObjectPtr Object);
};


class UClass : public UStruct
{
public:

	friend class UObject;
};

// shoutout to LiveDataminer iykyk =)