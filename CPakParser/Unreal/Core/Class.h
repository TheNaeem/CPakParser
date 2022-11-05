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
	std::vector<FProperty*> Properties;

public:

	void SetSuper(UStructPtr Val);
	UStructPtr GetSuper();
	std::vector<FProperty*> GetProperties();

	void SerializeScriptProperties(FSharedAr Ar, UObjectPtr Object);
};


class UClass : public UStruct
{
public:

	friend class UObject;
};

// shoutout to LiveDataminer iykyk =)