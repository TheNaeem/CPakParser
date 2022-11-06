#include "SoftObjectProperty.h"

#include "Serialization/Archives.h"

TUniquePtr<IPropValue> FSoftObjectProperty::Serialize(FArchive& Archive)
{
	auto Ret = std::make_unique<Value>();
	Archive << Ret->Path;

	return Ret;
}