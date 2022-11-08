#pragma once

#include "Property.h"
#include "PropertyType.h"
#include <optional>

class IPropValue
{
public:

	virtual bool IsAcceptableType(EPropertyType Type) = 0;
	virtual void PlaceValue(EPropertyType Type, void* OutBuffer) = 0;

	template <typename T>
	std::optional<T> TryGetValue()
	{
		auto RuntimeType = GetPropertyType<T>();

		if (!IsAcceptableType(RuntimeType))
			return std::nullopt;

		T Val;
		PlaceValue(RuntimeType, &Val);

		return std::optional<T>(Val);
	}
};