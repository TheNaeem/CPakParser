#pragma once

class FIoChunkHash
{
public:

	friend unsigned __int32 hash_value(const FIoChunkHash& InChunkHash);

	friend class FArchive& operator<<(class FArchive& Ar, FIoChunkHash& ChunkHash);

	inline bool operator ==(const FIoChunkHash& Rhs) const;

	inline bool operator !=(const FIoChunkHash& Rhs) const
	{
		return !(*this == Rhs);
	}

private:

	unsigned __int8 Hash[32];
};