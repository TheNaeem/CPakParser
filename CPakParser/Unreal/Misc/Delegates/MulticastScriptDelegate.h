#pragma once

#include "ScriptDelegate.h"
#include <vector>

class FMulticastScriptDelegate
{
	__forceinline std::vector<FScriptDelegate> GetInvocationList()
	{
		return InvocationList;
	}

	friend class FArchive& operator<<(FArchive& Ar, FMulticastScriptDelegate& Delegate);

protected:

	std::vector<FScriptDelegate> InvocationList;
};