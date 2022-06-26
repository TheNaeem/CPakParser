#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Enums.h"

#define MIN_int32 ((int32_t)0x80000000)
#define MAX_int32 ((int32_t)0x7fffffff)
#define MAX_int64 ((int64_t)0x7fffffffffffffff)
#define MAX_uint64 ((uint64_t)0xffffffffffffffff)
#define NAME_None 0
#define NETWORK_ORDER16(x) _byteswap_ushort(x)

typedef uint32_t FNameEntryId;

inline int32_t GIoDispatcherBufferSizeKB = 256;
enum { INDEX_NONE = -1 };

template <typename T> struct TCanBulkSerialize { enum { Value = false }; };
template<> struct TCanBulkSerialize<unsigned int> { enum { Value = true }; };
template<> struct TCanBulkSerialize<unsigned short> { enum { Value = true }; };
template<> struct TCanBulkSerialize<int> { enum { Value = true }; };

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

	FGuid() : 
		A(0),
		B(0),
		C(0), 
		D(0)
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
	};

	FName(std::string& InString) //later
	{
	};

	FName(uint32_t InValue) : Value(InValue)
	{
	};

	std::string ToString() //later
	{
	}
};

struct FField;

struct FFieldVariant
{
	union FFieldObjectUnion
	{
		FField* Field;
		UObject* Object;
	}Container;

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

class FProperty : public FField
{
public:
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

class IMappedFileRegion
{
	const uint8_t* MappedPtr;
	size_t MappedSize;
	std::string DebugFilename;
	size_t DebugOffsetRelativeToFile;

	__forceinline void CheckInvariants()
	{
	}

public:

	__forceinline IMappedFileRegion(const uint8_t* InMappedPtr, size_t InMappedSize, const std::string& InDebugFilename, size_t InDebugOffsetRelativeToFile)
		: MappedPtr(InMappedPtr)
		, MappedSize(InMappedSize)
		, DebugFilename(InDebugFilename)
		, DebugOffsetRelativeToFile(InDebugOffsetRelativeToFile)
	{
		CheckInvariants();
	}

	virtual ~IMappedFileRegion()
	{
	}

	__forceinline const uint8_t* GetMappedPtr()
	{
		CheckInvariants();
		return MappedPtr;
	}

	__forceinline int64_t GetMappedSize()
	{
		CheckInvariants();
		return MappedSize;
	}

	virtual void PreloadHint(int64_t PreloadOffset = 0, int64_t BytesToPreload = MAX_int64)
	{
	}

	IMappedFileRegion(const IMappedFileRegion&) = delete;
	IMappedFileRegion& operator=(const IMappedFileRegion&) = delete;
};

class IMappedFileHandle
{
	size_t MappedFileSize;

public:
	IMappedFileHandle(size_t InFileSize)
		: MappedFileSize(InFileSize)
	{
	}

	virtual ~IMappedFileHandle()
	{
	}

	int64_t GetFileSize()
	{
		return MappedFileSize;
	}

	virtual IMappedFileRegion* MapRegion(int64_t Offset = 0, int64_t BytesToMap = MAX_int64, bool bPreloadHint = false) = 0;

	// Non-copyable
	IMappedFileHandle(const IMappedFileHandle&) = delete;
	IMappedFileHandle& operator=(const IMappedFileHandle&) = delete;
};

class ReadStatus //TODO: log from the constructor 
{
public:
	ReadStatus(EIoErrorCode Code, std::string InErrorMessage) : ErrorCode(Code), ErrorMessage(InErrorMessage)
	{
	}

	ReadStatus(EIoErrorCode Code) : ErrorCode(Code)
	{
	}

	ReadStatus& operator=(const ReadStatus& Other);
	ReadStatus& operator=(const EIoErrorCode InErrorCode);

	bool operator==(const ReadStatus& Other) const;
	bool operator!=(const ReadStatus& Other) const { return !operator==(Other); }

	inline bool	IsOk() const { return ErrorCode == EIoErrorCode::Ok; }
	inline bool	IsCompleted() const { return ErrorCode != EIoErrorCode::Unknown; }
	inline EIoErrorCode	GetErrorCode() const { return ErrorCode; }
	std::string	ToString() { return ErrorMessage; }

	static const ReadStatus Ok;
	static const ReadStatus Unknown;
	static const ReadStatus Invalid;

private:
	EIoErrorCode ErrorCode = EIoErrorCode::Ok;
	std::string ErrorMessage;
};

template<typename Enum>
constexpr bool EnumHasAnyFlags(Enum Flags, Enum Contains)
{
	return (((__underlying_type(Enum))Flags) & (__underlying_type(Enum))Contains) != 0;
}

std::vector<std::string> LoadNameBatch(FArchive& Ar);