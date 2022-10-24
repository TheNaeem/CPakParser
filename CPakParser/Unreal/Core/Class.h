#pragma once

#include "UObject.h"

class UStruct : public UObject, public std::enable_shared_from_this<UStruct>
{
public:

	friend class UObject;

private:

	UStructPtr Super;

public:

	void SetSuper(UStructPtr Val);
	UStructPtr GetSuper();

	void SerializeScriptProperties(TSharedPtr<class FExportReader> Ar, uint8_t* Data, UStructPtr DefaultsStruct, uint8_t* Defaults);
};


class UClass : public UStruct
{
public:

	friend class UObject;
};

// shoutout to LiveDataminer iykyk =)