#include "ObjectProperty.h"

#include "Serialization/Archives.h"

TUniquePtr<IPropValue> FObjectProperty::Serialize(FArchive& Ar)
{
	auto Ret = std::make_unique<FObjectProperty::Value>();
	Ar << Ret->Object;

	return std::move(Ret);
}