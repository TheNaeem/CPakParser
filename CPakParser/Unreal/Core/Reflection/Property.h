#pragma once

#include "../Defines.h"

#include <string>

/*
* I made the decision to use raw pointers with properties.
* Mainly to avoid the overhead of using a shared pointer.
* Also, properties should only live as long as their parent class, which handles freeing them.
*/

enum class EPropertyType : uint8_t
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

class FProperty
{
public:

	friend class FPropertyFactory;
	friend struct FUnversionedSerializer;
	friend class Mappings;

	virtual ~FProperty() = default;

protected:

	std::string Name;
	uint16_t Index;
	uint8_t ArrayDim;
	EPropertyType Type;
	FProperty* Next = nullptr;

public:

	__forceinline std::string GetName()
	{
		return Name;
	}

	__forceinline uint16_t GetIndex()
	{
		return Index;
	}

	__forceinline uint8_t GetArrayDim()
	{
		return ArrayDim;
	}

	__forceinline FProperty* GetNext()
	{
		return Next;
	}

	virtual TUniquePtr<class IPropValue> Serialize(FArchive& Ar) 
	{
		return nullptr;
	}
};