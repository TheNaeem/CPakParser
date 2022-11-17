export module CPakParser.Reflection.PropertyType;

import <string>;

#define RUNTIME_TYPE(type, propType) if constexpr (std::is_same<T, type>()) return propType

export enum class EPropertyType : uint8_t
{
	ByteProperty,
	BoolProperty,
	IntProperty,
	FloatProperty,
	ObjectProperty,
	NameProperty,
	DelegateProperty,
	DoubleProperty,
	ArrayProperty,
	StructProperty,
	StrProperty,
	TextProperty,
	InterfaceProperty,
	MulticastDelegateProperty,
	WeakObjectProperty,
	LazyObjectProperty,
	AssetObjectProperty,
	SoftObjectProperty,
	UInt64Property,
	UInt32Property,
	UInt16Property,
	Int64Property,
	Int16Property,
	Int8Property,
	MapProperty,
	SetProperty,
	EnumProperty,
	FieldPathProperty,

	Unknown = 0xFF
};

export template <typename T>
constexpr EPropertyType GetPropertyType()
{
	RUNTIME_TYPE(uint8_t, EPropertyType::ByteProperty);
	RUNTIME_TYPE(bool, EPropertyType::BoolProperty);
	RUNTIME_TYPE(int32_t, EPropertyType::IntProperty);
	RUNTIME_TYPE(float, EPropertyType::FloatProperty);
	RUNTIME_TYPE(class UObjectPtr, EPropertyType::ObjectProperty);
	//RUNTIME_TYPE(class FName, EPropertyType::NameProperty);
	//RUNTIME_TYPE(FScriptDelegate, EPropertyType::DelegateProperty);
	RUNTIME_TYPE(double, EPropertyType::DoubleProperty);
	RUNTIME_TYPE(std::string, EPropertyType::StrProperty);
	//RUNTIME_TYPE(class FText, EPropertyType::TextProperty);
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