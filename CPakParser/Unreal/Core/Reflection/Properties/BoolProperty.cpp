#include "BoolProperty.h"

#include "Serialization/Archives.h"

TUniquePtr<IPropValue> FBoolProperty::Serialize(FArchive& Ar)
{
	auto Ret = std::make_unique<Value>();

	uint8_t ByteValue;
	Ar << ByteValue;

	Ret->Val = ByteValue;

	return std::move(Ret);
}