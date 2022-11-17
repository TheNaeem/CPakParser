module;

#include "Core/Defines.h"

export module CPakParser.Reflection.TextProperty;

import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;
import CPakParser.Localization.Text;

export class FTextProperty : public FProperty
{
public:

	class Value : public IPropValue
	{
	public:

		FText Text;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return false; // TODO: struct stuff
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			// TODO:
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
	{
		auto Ret = std::make_unique<Value>();
		Ar << Ret->Text;

		return std::move(Ret);
	}
};