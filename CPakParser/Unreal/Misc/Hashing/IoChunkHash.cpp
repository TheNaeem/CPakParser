#include "IoChunkHash.h"

#include "Serialization/Archives.h"

unsigned __int32 hash_value(const FIoChunkHash& InChunkHash)
{
	uint32_t Result = 5381;
	for (int i = 0; i < sizeof InChunkHash.Hash; ++i)
	{
		Result = Result * 33 + InChunkHash.Hash[i];
	}
	return Result;
}

FArchive& operator<<(FArchive& Ar, FIoChunkHash& ChunkHash)
{
	Ar.Serialize(&ChunkHash.Hash, sizeof(ChunkHash.Hash));
	return Ar;
}

bool FIoChunkHash::operator ==(const FIoChunkHash& Rhs) const
{
	return 0 == memcmp(Hash, Rhs.Hash, sizeof Hash);
}