#pragma once

#include <cstdint>

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

	bool operator ==(const FIoChunkId& Rhs) const;

	__forceinline bool operator !=(const FIoChunkId& Rhs) const
	{
		return !(*this == Rhs);
	}

	void Set(const void* InIdPtr, size_t InSize);
	bool IsValid() const;

	inline const uint8_t* GetData() const { return Id; }
	inline uint32_t	GetSize() const { return sizeof Id; }
	EIoChunkType GetChunkType() const;
	friend size_t hash_value(FIoChunkId const& in);

private:

	uint8_t Id[12] = { 0 };
};