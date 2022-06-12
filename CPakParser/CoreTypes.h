#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Enums.h"

#define MIN_int32 ((int32_t)0x80000000)
#define MAX_int32 ((int32_t)0x7fffffff)
#define NAME_None 0

typedef uint32_t FNameEntryId;

class UObject
{

};

template <class TEnum>
class TEnumAsByte
{
public:
	TEnumAsByte()
	{
	}

	TEnumAsByte(TEnum _value) : value(static_cast<uint8_t>(_value))
	{
	}

	explicit TEnumAsByte(int32_t _value) : value(static_cast<uint8_t>(_value))
	{
	}

	explicit TEnumAsByte(uint8_t _value) : value(_value)
	{
	}

	operator TEnum() const
	{
		return static_cast<TEnum>(value);
	}

	TEnum GetValue() const
	{
		return static_cast<TEnum>(value);
	}

private:
	uint8_t value;
};

class FNoncopyable
{
protected:
	FNoncopyable() {}
	~FNoncopyable() {}
private:
	FNoncopyable(const FNoncopyable&);
	FNoncopyable& operator=(const FNoncopyable&);
};


struct FGuid
{
	int32_t A;
	int32_t B;
	int32_t C;
	int32_t D;

	FGuid()
	{
	}

	FGuid(std::string);

	bool operator==(FGuid Other) const
	{
		return Other.A == A && Other.B == B && Other.C == C && Other.D == D;
	}

	bool operator!=(FGuid Other) const
	{
		return !(*this == Other);
	}
};

class FName
{
	FNameEntryId Value;

public:

	FName()
	{
	}

	FName(uint32_t InValue) : Value(InValue)
	{
	}
};

struct FFieldVariant
{
	union FFieldObjectUnion
	{
		FField* Field;
		UObject* Object;
	} Container;

	bool bIsUObject;
};

struct FFieldClass
{
	FName Name;
	FFieldClassID Id;
	unsigned __int64 CastFlags;
	EClassFlags ClassFlags;
	FFieldClass* SuperClass;
	FField* DefaultObject;
	void* pad;
	volatile int UnqiueNameIndexCounter;
};

struct FField
{
	void* __vftable;
	FFieldClass* ClassPrivate;
	FFieldVariant Owner;
	FField* Next;
	FName NamePrivate;
	EObjectFlags FlagsPrivate;
};

struct FProperty : FField
{
	int32_t ArrayDim;
	int32_t ElementSize;
	EPropertyFlags PropertyFlags;
	uint16_t RepIndex;
	TEnumAsByte<ELifetimeCondition> BlueprintReplicationCondition;
	int Offset_Internal;
	FName RepNotifyFunc;
	FProperty* PropertyLinkNext;
	FProperty* NextRef;
	FProperty* DestructorLinkNext;
	FProperty* PostConstructLinkNext;

	__forceinline bool HasAnyPropertyFlags(uint64_t FlagsToCheck) const
	{
		return (PropertyFlags & FlagsToCheck) != 0 || FlagsToCheck == 0xFFFFFFFFFFFFFFFF;
	}
};

class FArchive;

class FPackageId
{
	static constexpr uint64_t InvalidId = 0;
	uint64_t Id = InvalidId;

	inline explicit FPackageId(uint64_t InId) : Id(InId) {}

public:
	FPackageId() = default;

	static FPackageId FromName(const FName& Name);

	static FPackageId FromValue(const uint64_t Value)
	{
		return FPackageId(Value);
	}

	inline bool IsValid() const
	{
		return Id != InvalidId;
	}

	inline uint64_t Value() const
	{
		return Id;
	}

	inline uint64_t ValueForDebugging() const
	{
		return Id;
	}

	inline bool operator<(FPackageId Other) const
	{
		return Id < Other.Id;
	}

	inline bool operator==(FPackageId Other) const
	{
		return Id == Other.Id;
	}

	inline bool operator!=(FPackageId Other) const
	{
		return Id != Other.Id;
	}

	inline friend uint64_t GetTypeHash(const FPackageId& In)
	{
		return uint64_t(In.Id);
	}

	friend FArchive& operator<<(FArchive& Ar, FPackageId& Value);
};

class FDisplayNameEntryId
{
	FNameEntryId Id;
};

class FMappedName
{
	static constexpr uint32_t InvalidIndex = ~uint32_t(0);
	static constexpr uint32_t IndexBits = 30u;
	static constexpr uint32_t IndexMask = (1u << IndexBits) - 1u;
	static constexpr uint32_t TypeMask = ~IndexMask;
	static constexpr uint32_t TypeShift = IndexBits;

public:
	enum class EType
	{
		Package,
		Container,
		Global
	};

	inline FMappedName() = default;

	static inline FMappedName Create(const uint32_t InIndex, const uint32_t InNumber, EType InType)
	{
		return FMappedName((uint32_t(InType) << TypeShift) | InIndex, InNumber);
	}

	inline bool IsValid() const
	{
		return Index != InvalidIndex && Number != InvalidIndex;
	}

	inline EType GetType() const
	{
		return static_cast<EType>(uint32_t((Index & TypeMask) >> TypeShift));
	}

	inline bool IsGlobal() const
	{
		return ((Index & TypeMask) >> TypeShift) != 0;
	}

	inline uint32_t GetIndex() const
	{
		return Index & IndexMask;
	}

	inline uint32_t GetNumber() const
	{
		return Number;
	}

	inline bool operator!=(FMappedName Other) const
	{
		return Index != Other.Index || Number != Other.Number;
	}

	friend FArchive& operator<<(FArchive& Ar, FMappedName& MappedName);

private:
	inline FMappedName(const uint32_t InIndex, const uint32_t InNumber)
		: Index(InIndex)
		, Number(InNumber) { }

	uint32_t Index = InvalidIndex;
	uint32_t Number = InvalidIndex;
};