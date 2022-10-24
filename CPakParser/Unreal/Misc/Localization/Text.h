#pragma once

#include <string>

class FArchive;

enum class ELocResVersion : uint8_t
{
	Legacy = 0,
	Compact,
	Optimized_CRC32,
	Optimized_CityHash64_UTF16,

	LatestPlusOne,
	Latest = LatestPlusOne - 1
};

struct FTextLocalizationResourceString
{
	FTextLocalizationResourceString() = default;

	FTextLocalizationResourceString(std::string& InString, int32_t InRefCount = 0)
		: String(InString)
	{
	}

	std::string String;

	friend FArchive& operator<<(FArchive& Ar, FTextLocalizationResourceString& A);
};

class FTextKey
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

	void Serialize(FArchive& Ar, ELocResVersion& Ver);

private:

	std::string Str;
	uint32_t StrHash;
};

struct FTextId
{
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