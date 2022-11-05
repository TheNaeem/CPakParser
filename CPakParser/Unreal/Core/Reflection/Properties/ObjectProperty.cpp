#include "ObjectProperty.h"

#include "Serialization/Archives.h"

TUniquePtr<IPropValue> FObjectProperty::Serialize(FSharedAr Ar)
{
	auto Ret = std::make_unique<FObjectProperty::Value>();
	*Ar << Ret->Object;

	return Ret;
}