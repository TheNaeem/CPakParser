module;

#include "Core/Defines.h"

export module CPakParser.Reflection.BoolProperty;

export import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;

class FBoolProperty : public FProperty
{
public:

	struct Value : public IPropValue
	{
		bool Val = false;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::BoolProperty;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			memcpy(OutBuffer, &Val, sizeof(Val));
		}
	};

	TUniquePtr<IPropValue> Serialize(class FArchive& Ar) override
	{
		auto Ret = std::make_unique<Value>();

		uint8_t ByteValue;
		Ar << ByteValue;

		Ret->Val = ByteValue;

		return std::move(Ret);
	}
};