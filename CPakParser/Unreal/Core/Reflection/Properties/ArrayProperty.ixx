module;

#include "Core/Defines.h"

export module CPakParser.Reflection.ArrayProperty;

export import CPakParser.Reflection.FProperty;
import CPakParser.Serialization.FArchive;
import <vector>;

export class FArrayProperty : public FProperty
{
public:

	friend class FPropertyFactory;

	struct Value : public IPropValue
	{
	public:

		std::vector<TUniquePtr<class IPropValue>> Array;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == EPropertyType::ArrayProperty;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			// TODO:
		}
	};

	TUniquePtr<class IPropValue> Serialize(FArchive& Ar) override
	{
		auto Ret = std::make_unique<FArrayProperty::Value>();

		if (Ar.UseUnversionedPropertySerialization())
		{
			int32_t ArrayCount;
			Ar << ArrayCount;

			Ret->Array.resize(ArrayCount);

			for (size_t i = 0; i < ArrayCount; i++)
				Ret->Array[i] = ElementType->Serialize(Ar);

			return std::move(Ret);
		}

		return nullptr; // TODO: property tag crap
	}

private:

	FProperty* ElementType;

public:

	__forceinline FProperty* GetElementType()
	{
		return ElementType;
	}
};