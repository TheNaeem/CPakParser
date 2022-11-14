export module IoChunkHash;

import FArchiveBase;
import <cstdint>;

export class FIoChunkHash
{
public:

	friend unsigned __int32 hash_value(const FIoChunkHash& InChunkHash)
	{
		uint32_t Result = 5381;
		for (int i = 0; i < sizeof InChunkHash.Hash; ++i)
		{
			Result = Result * 33 + InChunkHash.Hash[i];
		}
		return Result;
	}

	inline friend FArchive& operator<<(FArchive& Ar, FIoChunkHash& ChunkHash)
	{
		Ar.Serialize(&ChunkHash.Hash, sizeof(ChunkHash.Hash));
		return Ar;
	}

	inline bool operator ==(const FIoChunkHash& Rhs) const
	{
		return 0 == memcmp(Hash, Rhs.Hash, sizeof Hash);
	}

	inline bool operator !=(const FIoChunkHash& Rhs) const
	{
		return !(*this == Rhs);
	}

private:

	unsigned __int8 Hash[32];
};