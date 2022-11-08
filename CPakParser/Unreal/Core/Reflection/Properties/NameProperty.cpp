#include "NameProperty.h"

#include "Serialization/Archives.h"

TUniquePtr<IPropValue> FNameProperty::Serialize(FArchive& Ar)
{
	auto Ret = std::make_unique<Value>();
	Ar << Ret->Name;

	return std::move(Ret);
}