#pragma once

#include "Property.h"
#include "../UObject.h"

#define RUNTIME_TYPE(type, propType) if constexpr (std::is_same<T, type>()) return propType

template <typename T>
constexpr EPropertyType GetPropertyType() // TODO: add everything
{
	RUNTIME_TYPE(std::string, EPropertyType::StrProperty);
	RUNTIME_TYPE(uint8_t, EPropertyType::ByteProperty);
	RUNTIME_TYPE(bool, EPropertyType::BoolProperty);
	RUNTIME_TYPE(int, EPropertyType::IntProperty);
	RUNTIME_TYPE(float, EPropertyType::FloatProperty);
	RUNTIME_TYPE(UObjectPtr, EPropertyType::ObjectProperty);

	

	return EPropertyType::Unknown;
}