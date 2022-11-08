#include "EnumProperty.h"

#include "Serialization/Archives.h"

TUniquePtr<IPropValue> FEnumProperty::Serialize(FArchive& Ar)
{
	auto Ret = std::make_unique<Value>();

	if (Enum and UnderlyingProp)
	{
		auto EnumVal = UnderlyingProp->Serialize(Ar);
		auto IntValOpt = EnumVal->TryGetValue<int64_t>();

		if (!IntValOpt.has_value())
			return nullptr;

		auto EnumIndex = IntValOpt.value();

		if (EnumIndex >= Enum->Enum.size())
			return nullptr;

		Ret->EnumName = Enum->Enum[EnumIndex];
	}

	return std::move(Ret);
}