module;

#include "Core/Defines.h"

export module CPakParser.Localization.Text;

export import <string>;
import CPakParser.Serialization.FArchive;

export enum class ELocResVersion : uint8_t
{
	Legacy = 0,
	Compact,
	Optimized_CRC32,
	Optimized_CityHash64_UTF16,

	LatestPlusOne,
	Latest = LatestPlusOne - 1
};

export enum class ETextGender : uint8_t
{
	Masculine,
	Feminine,
	Neuter
};

export struct FTextLocalizationResourceString
{
	FTextLocalizationResourceString() = default;

	FTextLocalizationResourceString(std::string& InString, int32_t InRefCount = 0)
		: String(InString)
	{
	}

	std::string String;

	friend FArchive& operator<<(FArchive& Ar, FTextLocalizationResourceString& A);
};

export class FTextKey
{
public:

	friend __forceinline bool operator==(const FTextKey& A, const FTextKey& B)
	{
		return A.Str == B.Str;
	}

	friend __forceinline bool operator!=(const FTextKey& A, const FTextKey& B)
	{
		return A.Str != B.Str;
	}

	friend size_t hash_value(const FTextKey& i)
	{
		return i.StrHash;
	}

	__forceinline std::string ToString() const
	{
		return Str;
	}

	void SerializeAsString(FArchive& Ar);
	void Serialize(FArchive& Ar, ELocResVersion& Ver);

private:

	std::string Str;
	uint32_t StrHash;
};

export struct FTextId
{
	FTextId() = default;

	FTextId(const FTextKey& InNamespace, const FTextKey& InKey)
		: Namespace(InNamespace)
		, Key(InKey)
	{
	}

	friend __forceinline bool operator==(const FTextId& A, const FTextId& B)
	{
		return A.Namespace == B.Namespace && A.Key == B.Key;
	}

	friend __forceinline bool operator!=(const FTextId& A, const FTextId& B)
	{
		return A.Namespace != B.Namespace || A.Key != B.Key;
	}

	friend size_t hash_value(const FTextId& i);

	FTextKey Namespace;
	FTextKey Key;
};

export class ITextData
{
public:

	virtual std::string& GetString() = 0;
	virtual void Serialize(class FArchive& Ar) = 0;
};

export class FText
{
public:

	FText() = default;

	__forceinline std::string& GetCultureInvariantString()
	{
		return CultureInvariantString;
	}

	__forceinline int32_t GetFlags()
	{
		return Flags;
	}

	__forceinline std::string& ToString()
	{
		return Data->GetString();
	}

	friend FArchive& operator<<(FArchive& Ar, FText& Value);

private:

	int32_t Flags;
	std::string CultureInvariantString;
	TUniquePtr<ITextData> Data;
};