#include "MulticastDelegateProperty.h"
#include "DelegateProperty.h"

#include "Serialization/Archives.h"

TUniquePtr<IPropValue> FMulticastDelegateProperty::Serialize(FSharedAr Ar)
{
	auto Ret = std::make_unique<Value>();
	*Ar << Ret->Delegate;
	
	return Ret;
}

TUniquePtr<IPropValue> FDelegateProperty::Serialize(FSharedAr Ar)
{
	auto Ret = std::make_unique<Value>();
	*Ar << Ret->Delegate;

	return Ret;
}