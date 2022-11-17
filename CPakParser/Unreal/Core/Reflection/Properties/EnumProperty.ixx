module;

#include "Core/Defines.h"

export module CPakParser.Reflection.EnumProperty;

import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;
import <string>;
import <vector>;

export struct FReflectedEnum // traits that can be shared across multiple instances of the same enum property
{
	std::vector<std::string> Enum;
	std::string EnumName;
};

export class FEnumProperty : public FProperty
{
public:

	friend class FPropertyFactory;

	struct Value : public IPropValue
	{
		int64_t BinaryValue = 0;
		std::string EnumName;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::EnumProperty;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
		}
	};

private:

	TSharedPtr<FReflectedEnum> Enum;
	FProperty* UnderlyingProp;

public:

	__forceinline FProperty* GetUnderlying()
	{
		return UnderlyingProp;
	}

	__forceinline std::vector<std::string> GetValues()
	{
		return Enum->Enum;
	}

	__forceinline std::string GetEnumName()
	{
		return Enum->EnumName;
	}

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
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
};