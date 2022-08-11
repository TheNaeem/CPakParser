#pragma once

#include <cstdint>
#include <Windows.h>

enum class EIoChunkType : uint8_t
{
	Invalid = 0,
	ExportBundleData = 1,
	BulkData = 2,
	OptionalBulkData = 3,
	MemoryMappedBulkData = 4,
	ScriptObjects = 5,
	ContainerHeader = 6,
	ExternalFile = 7,
	ShaderCodeLibrary = 8,
	ShaderCode = 9,
	PackageStoreEntry = 10,
	DerivedData = 11,
	EditorDerivedData = 12,

	MAX
};

class FIoChunkId
{
public:
	FIoChunkId() = default;
	FIoChunkId(uint64_t ChunkId, uint16_t ChunkIndex, EIoChunkType IoChunkType);

	static const FIoChunkId InvalidChunkId;

	__forceinline bool operator ==(const FIoChunkId& Rhs) const
	{
		return 0 == memcmp(Id, Rhs.Id, sizeof Id);
	}

	__forceinline bool operator !=(const FIoChunkId& Rhs) const
	{
		return !(*this == Rhs);
	}

	void Set(const void* InIdPtr, size_t InSize)
	{
		memcpy(Id, InIdPtr, sizeof Id);
	}

	inline bool IsValid() const
	{
		return *this != InvalidChunkId;
	}

	inline const uint8_t* GetData() const { return Id; }
	inline uint32_t	GetSize() const { return sizeof Id; }

	EIoChunkType GetChunkType() const
	{
		return static_cast<EIoChunkType>(Id[11]);
	}

	friend size_t hash_value(FIoChunkId const& in)
	{
		uint32_t Hash = 5381;

		for (int i = 0; i < sizeof Id; ++i)
		{
			Hash = Hash * 33 + in.Id[i];
		}

		return Hash;
	}

private:
	static inline FIoChunkId CreateEmptyId()
	{
		FIoChunkId ChunkId;
		uint8_t Data[12] = { 0 };
		ChunkId.Set(Data, sizeof Data);

		return ChunkId;
	}

	uint8_t Id[12] = { 0 };
};