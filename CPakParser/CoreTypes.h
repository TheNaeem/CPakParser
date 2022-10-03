#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "Enums.h"
#include "AES.h"
#include "../Dependencies/parallel_hashmap/phmap.h"

#define LOGGING 1

#define SCOPE_LOCK(InLock) std::lock_guard<std::mutex> _(InLock);

#define MIN_int32 ((int32_t)0x80000000)
#define MAX_int32 ((int32_t)0x7fffffff)
#define MAX_int64 ((int64_t)0x7fffffffffffffff)
#define MAX_uint64 ((uint64_t)0xffffffffffffffff)
#define NAME_None 0

typedef uint32_t FNameEntryId;

constexpr int32_t GIoDispatcherBufferSizeKB = 256;
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

	constexpr FGuid(uint32_t InA, uint32_t InB, uint32_t InC, uint32_t InD)
		: A(InA), B(InB), C(InC), D(InD)
	{ }

	FGuid() : 
		A(0),
		B(0),
		C(0), 
		D(0)
	{
	}

	FGuid(std::string);

	friend class FArchive& operator<<(class FArchive& Ar, FGuid& Value);

	bool operator==(FGuid Other) const
	{
		return Other.A == A && Other.B == B && Other.C == C && Other.D == D;
	}

	bool operator!=(FGuid Other) const
	{
		return !(*this == Other);
	}

	bool IsValid() const
	{
		return ((A | B | C | D) != 0);
	}

	void Invalidate()
	{
		A = B = C = D = 0;
	}

	friend size_t hash_value(const FGuid& Guid) 
	{
		return 
			std::hash<int32_t>{}(Guid.A) ^ 
			std::hash<int32_t>{}(Guid.B) ^ 
			std::hash<int32_t>{}(Guid.C) ^ 
			std::hash<int32_t>{}(Guid.D);
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

	FPackageId(const std::string& Name);

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

	__forceinline friend size_t hash_value(const FPackageId& In)
	{
		return In.Id;
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

	static __forceinline FMappedName Create(const uint32_t InIndex, const uint32_t InNumber, EType InType)
	{
		return FMappedName((uint32_t(InType) << TypeShift) | InIndex, InNumber);
	}

	__forceinline bool IsValid() const
	{
		return Index != InvalidIndex && Number != InvalidIndex;
	}

	__forceinline EType GetType() const
	{
		return static_cast<EType>(uint32_t((Index & TypeMask) >> TypeShift));
	}

	__forceinline bool IsGlobal() const
	{
		return ((Index & TypeMask) >> TypeShift) != 0;
	}

	__forceinline uint32_t GetIndex() const
	{
		return Index & IndexMask;
	}

	__forceinline uint32_t GetNumber() const
	{
		return Number;
	}

	__forceinline bool operator!=(FMappedName Other) const
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

class FNameMap
{
public:

	__forceinline int32_t Num() const
	{
		return NameEntries.size();
	}

	__forceinline const std::string& GetName(FMappedName& MappedName) const
	{
		return NameEntries[MappedName.GetIndex()];
	}

	void Serialize(class FArchive& Ar, FMappedName::EType NameMapType);

private:

	std::vector<std::string> NameEntries;
	FMappedName::EType NameMapType = FMappedName::EType::Global;
};

class IDiskFile
{
public:

	virtual std::filesystem::path GetDiskPath() = 0;
	virtual std::unique_ptr<class FArchive> CreateEntryArchive(struct FFileEntryInfo EntryInfo) = 0;
	virtual void DoWork(std::unique_ptr<class FArchive>& Ar) = 0;
};

struct FFileEntryInfo
{
	FFileEntryInfo()
	{
		Entry.TocIndex = NULL;
	};

	FFileEntryInfo(uint32_t InIndex) { Entry.TocIndex = InIndex; }
	FFileEntryInfo(int32_t InIndex) { Entry.PakIndex = InIndex; }

	friend FArchive& operator<<(FArchive& Ar, FFileEntryInfo& EntryInfo);

	__forceinline bool IsValid() 
	{
		return Entry.PakIndex;
	}

	__forceinline void SetOwningFile(std::shared_ptr<IDiskFile> DiskFile)
	{
		AssociatedFile = DiskFile;
	}

	__forceinline std::filesystem::path GetDiskFilePath()
	{
		return AssociatedFile->GetDiskPath();
	}

	__forceinline std::shared_ptr<IDiskFile> GetAssociatedFile()
	{
		return AssociatedFile;
	}

	__forceinline uint32_t GetTocIndex()
	{
		return Entry.TocIndex;
	}

	__forceinline int32_t GetPakIndex()
	{
		return Entry.PakIndex;
	}

protected:

	union 
	{
		int32_t PakIndex;
		uint32_t TocIndex;
	}Entry;

	std::shared_ptr<IDiskFile> AssociatedFile;
};

//TODO: proper logging from this
class ReadStatus //totally not an FIoStatus rip off
{
public:
	ReadStatus(ReadErrorCode Code, std::string InStatusMessage) : ErrorCode(Code), StatusMessage(InStatusMessage)
	{
#if LOGGING
		printf("%s\n", InStatusMessage.c_str());
#endif
	}

	ReadStatus(ReadErrorCode Code) : ErrorCode(Code)
	{
	}

	ReadStatus& operator=(const ReadStatus& Other);
	ReadStatus& operator=(const ReadErrorCode InErrorCode);

	bool operator==(const ReadStatus& Other) const;
	bool operator!=(const ReadStatus& Other) const { return !operator==(Other); }

	inline bool	IsOk() const { return ErrorCode == ReadErrorCode::Ok; }
	inline bool	IsCompleted() const { return ErrorCode != ReadErrorCode::Unknown; }
	inline ReadErrorCode	GetErrorCode() const { return ErrorCode; }
	std::string	ToString() { return StatusMessage; }

private:
	ReadErrorCode ErrorCode = ReadErrorCode::Ok;
	std::string StatusMessage;
};

class FEncryptionKeyManager
{
public:

	static FEncryptionKeyManager& Get()
	{
		static FEncryptionKeyManager Inst;
		return Inst;
	}

	static void AddKey(const FGuid& InGuid, const FAESKey InKey);
	static bool GetKey(const FGuid& InGuid, FAESKey& OutKey);
	static bool const HasKey(const FGuid& InGuid);
	static const phmap::flat_hash_map<FGuid, FAESKey>& GetKeys();

private:

	FEncryptionKeyManager() = default;

	phmap::flat_hash_map<FGuid, FAESKey> Keys;
	std::mutex CriticalSection;
};

template<typename Enum>
constexpr bool EnumHasAnyFlags(Enum Flags, Enum Contains)
{
	return (((__underlying_type(Enum))Flags) & (__underlying_type(Enum))Contains) != 0;
}

template <typename T>
static __forceinline constexpr T Align(T Val, uint64_t Alignment)
{
	return (T)(((uint64_t)Val + Alignment - 1) & ~(Alignment - 1));
}

std::vector<std::string> LoadNameBatch(FArchive& Ar);