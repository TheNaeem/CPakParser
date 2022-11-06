#pragma once

#include "../Property.h"
#include "../PropertyValue.h"
#include "Serialization/Archives.h"

template <typename NumType, EPropertyType NumPropType>
class TNumericProperty : public FProperty
{
public:

	class Value : public IPropValue
	{
	public:

		NumType Val;

		__forceinline bool IsAcceptableType(EPropertyType Type) override
		{
			return Type == NumPropType;
		}

		__forceinline void PlaceValue(EPropertyType Type, void* OutBuffer) override
		{
			if (Type == NumPropType)
				memcpy(OutBuffer, &Val, sizeof(NumType));
		}
	};

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
	{
		auto Ret = std::make_unique<Value>();
		Ar << Ret->Val;

		return Ret;
	}
};

typedef TNumericProperty<float, EPropertyType::FloatProperty> FFloatProperty;
typedef TNumericProperty<double, EPropertyType::DoubleProperty> FDoubleProperty;
typedef TNumericProperty<int8_t, EPropertyType::Int8Property> FInt8Property;
typedef TNumericProperty<int16_t, EPropertyType::Int16Property> FInt16Property;
typedef TNumericProperty<int32_t, EPropertyType::IntProperty> FIntProperty;
typedef TNumericProperty<int64_t, EPropertyType::Int64Property> FInt64Property;
typedef TNumericProperty<uint16_t, EPropertyType::UInt16Property> FUInt16Property;
typedef TNumericProperty<uint32_t, EPropertyType::UInt32Property> FUInt32Property;
typedef TNumericProperty<uint64_t, EPropertyType::UInt64Property> FUInt64Property;

class FByteProperty : public TNumericProperty<uint8_t, EPropertyType::ByteProperty>
{
public:

	TUniquePtr<IPropValue> Serialize(FArchive& Ar) override
	{// TODO:
		return nullptr;
	}
};