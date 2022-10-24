#pragma once

#include "Core/Defines.h"
#include "Misc/Hashing/Map.h"

class FArchive 
{
private:

	bool bUseUnversionedProperties = false;

public:

	virtual void Serialize(void* V, int64_t Length) { }

	virtual FArchive& operator<<(class FName& Value)
	{
		return *this;
	}

	template<class T1, class T2>
	friend FArchive& operator<<(FArchive& Ar, std::pair<T1, T2>& InPair)
	{
		Ar << InPair.first;
		Ar << InPair.second;

		return Ar;
	}

	template<class T>
	friend FArchive& operator<<(FArchive& Ar, phmap::flat_hash_set<T>& InSet)
	{
		int32_t NewNumElements = 0;
		Ar << NewNumElements;

		if (!NewNumElements) return Ar;

		InSet.clear();
		InSet.reserve(NewNumElements);

		for (size_t i = 0; i < NewNumElements; i++)
		{
			T Element;
			Ar << Element;
			InSet.insert(Element);
		}

		return Ar;
	}

	template<class Key, class Value>
	friend FArchive& operator<<(FArchive& Ar, TMap<Key, Value>& InMap)
	{
		auto Pairs = phmap::flat_hash_set<std::pair<Key, Value>>();

		int32_t NewNumElements = 0;
		Ar << NewNumElements;

		if (!NewNumElements) return Ar;

		InMap.reserve(NewNumElements);

		for (size_t i = 0; i < NewNumElements; i++)
		{
			auto Pair = std::pair<Key, Value>();

			Ar << Pair;

			InMap.insert_or_assign(Pair.first, Pair.second);
		}

		return Ar;
	}

	template<typename T>
	friend FArchive& operator<<(FArchive& Ar, std::vector<T>& InArray)
	{
		if constexpr (sizeof(T) == 1 || TCanBulkSerialize<T>::Value)
		{
			return Ar.BulkSerializeArray(InArray);
		}

		int32_t ArrayNum;
		Ar << ArrayNum;

		if (ArrayNum == 0)
		{
			InArray.clear();
			return Ar;
		}

		InArray.resize(ArrayNum);

		for (auto i = 0; i < InArray.size(); i++)
			Ar << InArray[i];

		return Ar;
	}

	template<typename T>
	__forceinline FArchive& BulkSerializeArray(std::vector<T>& InArray)
	{
		int32_t ArrayNum;
		*this << ArrayNum;

		if (ArrayNum == 0)
		{
			InArray.clear();
			return *this;
		}

		return BulkSerializeArray(InArray, ArrayNum);
	}

	template<typename T>
	FArchive& BulkSerializeArray(std::vector<T>& InArray, int32_t Count)
	{
		InArray.resize(Count);

		this->Serialize(InArray.data(), Count * sizeof(T));

		return *this;
	}

	template<typename T>
	__forceinline void BulkSerialize(void* V) // the idea here is to save time by reducing the amount of serialization operations done, but a few conditions have to be met before using this. i would just avoid this for now
	{
		Serialize(V, sizeof(T));
	}

	friend FArchive& operator<<(FArchive& Ar, std::string& InString);
	friend FArchive& operator<<(FArchive& Ar, int32_t& InNum);
	friend FArchive& operator<<(FArchive& Ar, uint32_t& InNum);
	friend FArchive& operator<<(FArchive& Ar, uint64_t& InNum);
	friend FArchive& operator<<(FArchive& Ar, int64_t& InNum);
	friend FArchive& operator<<(FArchive& Ar, uint8_t& InNum);
	friend FArchive& operator<<(FArchive& Ar, uint16_t& InNum);
	friend FArchive& operator<<(FArchive& Ar, int16_t& InNum);
	friend FArchive& operator<<(FArchive& Ar, bool& InBool);

	virtual void Seek(int64_t InPos) { }

	virtual void* Data() { return nullptr; }

	__forceinline void SeekCur(int64_t InAdvanceCount)
	{
		Seek(Tell() + InAdvanceCount);
	}

	template <typename T>
	__forceinline void SeekCur()
	{
		SeekCur(sizeof(T));
	}

	virtual int64_t Tell()
	{
		return INDEX_NONE;
	}

	virtual int64_t TotalSize()
	{
		return INDEX_NONE;
	}

	__forceinline bool UseUnversionedPropertySerialization()
	{
		return bUseUnversionedProperties;
	}

	__forceinline void SetUnversionedProperties(bool IsUsingUnversionedProperties)
	{
		bUseUnversionedProperties = IsUsingUnversionedProperties;
	}
};