#pragma once

#include "Property.h"
#include "../Names/Name.h"
#include "Misc/Localization/Text.h"

#define RUNTIME_TYPE(type, propType) if constexpr (std::is_same<T, type>()) return propType

template <typename T>
constexpr EPropertyType GetPropertyType()
{
	RUNTIME_TYPE(uint8_t, EPropertyType::ByteProperty);
	RUNTIME_TYPE(bool, EPropertyType::BoolProperty);
	RUNTIME_TYPE(int32_t, EPropertyType::IntProperty);
	RUNTIME_TYPE(float, EPropertyType::FloatProperty);
	RUNTIME_TYPE(class UObjectPtr, EPropertyType::ObjectProperty);
	RUNTIME_TYPE(FName, EPropertyType::NameProperty);
	//RUNTIME_TYPE(FScriptDelegate, EPropertyType::DelegateProperty);
	RUNTIME_TYPE(double, EPropertyType::DoubleProperty);
	RUNTIME_TYPE(std::string, EPropertyType::StrProperty);
	RUNTIME_TYPE(FText, EPropertyType::TextProperty);
	//RUNTIME_TYPE(FMulticastScriptDelegate, EPropertyType::MulticastDelegateProperty);
	RUNTIME_TYPE(uint64_t, EPropertyType::UInt64Property);
	RUNTIME_TYPE(uint32_t, EPropertyType::UInt32Property);
	RUNTIME_TYPE(uint16_t, EPropertyType::UInt16Property);
	RUNTIME_TYPE(int64_t, EPropertyType::Int64Property);
	RUNTIME_TYPE(int16_t, EPropertyType::Int16Property);
	RUNTIME_TYPE(int8_t, EPropertyType::Int8Property);

	// TODO: array, struct, map, set, enum, delegate, and field path support

	return EPropertyType::Unknown;
}