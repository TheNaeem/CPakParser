export module LazyPackageObject;

import CPakParser.Core.UObject;
import CPakParser.Package.Id;

export class ULazyPackageObject : public UObject
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