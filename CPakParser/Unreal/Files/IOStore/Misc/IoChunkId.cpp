#include "IoChunkId.h"

#include <stdlib.h>
#include <vcruntime_string.h>

#define NETWORK_ORDER16(x) _byteswap_ushort(x)

const FIoChunkId FIoChunkId::InvalidChunkId = FIoChunkId();

FIoChunkId::FIoChunkId(uint64_t ChunkId, uint16_t ChunkIndex, EIoChunkType IoChunkType)
{
	*reinterpret_cast<uint64_t*>(&Id[0]) = ChunkId;
	*reinterpret_cast<uint16_t*>(&Id[8]) = NETWORK_ORDER16(ChunkIndex);
	*reinterpret_cast<uint8_t*>(&Id[11]) = static_cast<uint8_t>(IoChunkType);
}

void FIoChunkId::Set(const void* InIdPtr, size_t InSize)
{
	memcpy(Id, InIdPtr, sizeof Id);
}

bool FIoChunkId::IsValid() const
{
	return *this != InvalidChunkId;
}

EIoChunkType FIoChunkId::GetChunkType() const
{
	return static_cast<EIoChunkType>(Id[11]);
}

size_t hash_value(FIoChunkId const& in)
{
	uint32_t Hash = 5381;

	for (int i = 0; i < sizeof in.Id; ++i)
	{
		Hash = Hash * 33 + in.Id[i];
	}

	return Hash;
}

bool FIoChunkId::operator ==(const FIoChunkId& Rhs) const
{
	return 0 == memcmp(Id, Rhs.Id, sizeof Id);
}