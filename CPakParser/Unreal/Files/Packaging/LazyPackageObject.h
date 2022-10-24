#pragma once

#include "Core/UObject.h"
#include "PackageId.h"

class ULazyPackageObject : public UObject
{
	FPackageId PackageId;

public:

	ULazyPackageObject(FPackageId InPackageId) : PackageId(InPackageId)
	{
		SetFlags(RF_NeedLoad);
	}

	void Load() override
	{
		// TODO: this

		ClearFlags(RF_NeedLoad);
	}
};