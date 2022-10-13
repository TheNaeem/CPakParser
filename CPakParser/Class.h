#pragma once

#include "UObject.h"

class UStruct : public UObject
{
public:

	friend class UObject;

private:

	UStructPtr Super;

public:

	void SetSuper(UStructPtr Val);
	UStructPtr GetSuper();
};


class UClass : public UStruct
{
public:

	friend class UObject;
};

// shoutout to LiveDataminer iykyk =)