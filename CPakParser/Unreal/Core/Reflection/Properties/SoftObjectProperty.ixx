module;

#include "Core/Defines.h"

export module CPakParser.Reflection.SoftObjectProperty;

import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;
import CPakParser.Paths.SoftObjectPath;

export class FSoftObjectProperty : public FProperty
{
public:

	class Value : public IPropValue
	{
	public:

		FSoftObjectPath Path;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::SoftObjectProperty or Type == EPropertyType::ObjectProperty;
		}

		void PlaceValue(EPropertyType Type, void* OutBuffer)
		{
			if (Type == EPropertyType::SoftObjectProperty)
			{
				*((FSoftObjectPath*)OutBuffer) = Path;
			}
			else if (Type == EPropertyType::ObjectProperty)
			{
				//TODO: 
			}
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Archive) override
	{
		auto Ret = std::make_unique<Value>();
		Archive << Ret->Path;

		return std::move(Ret);
	}
};