#include "EnumProperty.h"

#include "Serialization/Archives.h"

TUniquePtr<IPropValue> FEnumProperty::Serialize(FArchive& Ar)
{
	auto Ret = std::make_unique<Value>();

	if (Enum and UnderlyingProp)
	{
		auto EnumVal = UnderlyingProp->Serialize(Ar);
		auto IntVal = EnumVal->TryGetValue<int32_t>();
	}

	return Ret;
}