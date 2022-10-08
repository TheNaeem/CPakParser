#pragma once

#include "CoreTypes.h"
#include "Versioning.h"

struct FArchiveSerializedPropertyChain
{
public:
	/** Default constructor */
	FArchiveSerializedPropertyChain()
		: SerializedPropertyChainUpdateCount(0)
	{
	}

	void PushProperty(class FProperty* InProperty, const bool bIsEditorOnlyProperty)
	{
		SerializedPropertyChain.push_back(InProperty);
		IncrementUpdateCount();
	}

	void PopProperty(class FProperty* InProperty)
	{
		SerializedPropertyChain.pop_back();
		IncrementUpdateCount();
	}

	class FProperty* GetPropertyFromStack(const int32_t InStackIndex) const
	{
		return SerializedPropertyChain.rbegin()[InStackIndex];
	}

	class FProperty* GetPropertyFromRoot(const int32_t InRootIndex) const
	{
		return SerializedPropertyChain[InRootIndex];
	}

	uint32_t GetNumProperties() const
	{
		return uint32_t(SerializedPropertyChain.size());
	}

	uint32_t GetUpdateCount() const
	{
		return SerializedPropertyChainUpdateCount;
	}

private:
	void IncrementUpdateCount()
	{
		while (++SerializedPropertyChainUpdateCount == 0) {}
	}

	std::vector<class FProperty*> SerializedPropertyChain;
	uint32_t SerializedPropertyChainUpdateCount;
};

struct FArchiveState
{
private:
	friend class FArchive;

	FArchiveState();
	~FArchiveState();

public:

	virtual void Reset();

	virtual void CountBytes(size_t InNum, size_t InMax) { }

	virtual int64_t Tell()
	{
		return INDEX_NONE;
	}

	virtual int64_t TotalSize() 
	{
		return INDEX_NONE;
	}

	__forceinline void SetUnversionedProperties(bool IsUsingUnversionedProperties)
	{
		bUseUnversionedProperties = IsUsingUnversionedProperties;
	}

protected:

	bool bUseUnversionedProperties = false;
	int32_t ArSerializingDefaults;
	uint32_t ArPortFlags;
	int64_t ArMaxSerializeSize;
	FPackageFileVersion ArUEVer;
	FEngineVersionBase ArEngineVer;
	mutable FCustomVersionContainer* CustomVersionContainer = nullptr;
	FProperty* SerializedProperty;
	FArchiveSerializedPropertyChain* SerializedPropertyChain;
	mutable bool bCustomVersionsAreReset;

private:
	FArchiveState* NextProxy = nullptr;
};

class FArchive : public FArchiveState
{
public:

	virtual void Serialize(void* V, int64_t Length) { }

	virtual FArchive& operator<<(FName& Value)
	{
		return *this;
	}

	virtual FArchive& operator<<(UObject*& Value)
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
	friend FArchive& operator<<(FArchive& Ar, phmap::flat_hash_map<Key, Value>& InMap)
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
	__forceinline friend FArchive& operator<<(FArchive& Ar, int32_t& InNum);
	__forceinline friend FArchive& operator<<(FArchive& Ar, uint32_t& InNum);
	__forceinline friend FArchive& operator<<(FArchive& Ar, uint64_t& InNum);
	friend FArchive& operator<<(FArchive& Ar, int64_t& InNum);
	friend FArchive& operator<<(FArchive& Ar, uint8_t& InNum);
	friend FArchive& operator<<(FArchive& Ar, bool& InBool);

	virtual void Seek(int64_t InPos) { }

	__forceinline void SeekCur(int64_t InAdvanceCount)
	{
		Seek(Tell() + InAdvanceCount);
	}

	template <typename T>
	__forceinline void SeekCur()
	{
		SeekCur(sizeof(T));
	}
};

typedef TUniquePtr<FArchive> FUniqueAr;
typedef TSharedPtr<FArchive> FSharedAr;