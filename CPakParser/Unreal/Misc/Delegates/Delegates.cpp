#include "MulticastScriptDelegate.h"

#include "Serialization/Archives.h"

FArchive& operator<<(FArchive& Ar, FScriptDelegate& Delegate)
{
	return Ar << Delegate.Object << Delegate.FunctionName;
}

FArchive& operator<<(FArchive& Ar, FMulticastScriptDelegate& Delegate)
{
	return Ar << Delegate.InvocationList;
}