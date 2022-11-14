export module ScriptDelegate;

import UObjectCore;
import Name;
import FArchiveBase;

export class FScriptDelegate
{
	UObjectPtr Object;
	FName FunctionName;

public:

	friend class FArchive& operator<<(FArchive& Ar, FScriptDelegate& Delegate)
	{
		return Ar << Delegate.Object << Delegate.FunctionName;
	}

	__forceinline std::string GetFunctionName()
	{
		return FunctionName.ToString();
	}

	__forceinline UObjectPtr GetObject()
	{
		return Object;
	}
};