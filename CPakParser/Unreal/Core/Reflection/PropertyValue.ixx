export module PropertyValue;

import <optional>;

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

export class IPropValue
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