export module MulticastScriptDelegate;

import ScriptDelegate;
import FArchiveBase;
import <vector>;

export class FMulticastScriptDelegate
{
	__forceinline std::vector<FScriptDelegate> GetInvocationList()
	{
		return InvocationList;
	}

	friend class FArchive& operator<<(FArchive& Ar, FMulticastScriptDelegate& Delegate)
	{
		return Ar << Delegate.InvocationList;
	}

protected:

	std::vector<FScriptDelegate> InvocationList;
};