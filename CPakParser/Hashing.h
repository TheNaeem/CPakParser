#pragma once

class FSHAHash
{
public:
	alignas(uint32_t) uint8_t Hash[20];

	FSHAHash()
	{
		memset(Hash, 0, sizeof(Hash));
	}

	friend bool operator==(const FSHAHash& X, const FSHAHash& Y)
	{
		return memcmp(&X.Hash, &Y.Hash, sizeof(X.Hash)) == 0;
	}

	friend bool operator!=(const FSHAHash& X, const FSHAHash& Y)
	{
		return memcmp(&X.Hash, &Y.Hash, sizeof(X.Hash)) != 0;
	}

	friend bool operator<(const FSHAHash& X, const FSHAHash& Y)
	{
		return memcmp(&X.Hash, &Y.Hash, sizeof(X.Hash)) < 0;
	}

	friend FArchive& operator<<(FArchive& Ar, FSHAHash& G);

	friend uint32_t GetTypeHash(const FSHAHash& InKey)
	{
		return *reinterpret_cast<const uint32_t*>(InKey.Hash);
	}
};

struct FBlake3Hash
{
public:
	using ByteArray = uint8_t[32];

	FBlake3Hash() = default;

	inline ByteArray& GetBytes() { return Hash; }
	inline const ByteArray& GetBytes() const { return Hash; }

	/** A zero hash. */
	static const FBlake3Hash Zero;

private:
	alignas(uint32_t) ByteArray Hash{};
};


struct FIoHash
{
public:
	using ByteArray = uint8_t[20];

	FIoHash() = default;

	inline void Reset() { *this = FIoHash(); }

	inline ByteArray& GetBytes() { return Hash; }
	inline const ByteArray& GetBytes() const { return Hash; }

	static const FIoHash Zero;

private:
	alignas(uint32_t) ByteArray Hash{};
};

class FIoChunkHash
{
public:
	friend uint32_t GetTypeHash(const FIoChunkHash& InChunkHash)
	{
		uint32_t Result = 5381;
		for (int i = 0; i < sizeof Hash; ++i)
		{
			Result = Result * 33 + InChunkHash.Hash[i];
		}
		return Result;
	}

	friend FArchive& operator<<(FArchive& Ar, FIoChunkHash& ChunkHash);
	/*
	{
		Ar.Serialize(&ChunkHash.Hash, sizeof Hash);
		return Ar;
	}
	*/

	inline bool operator ==(const FIoChunkHash& Rhs) const
	{
		return 0 == memcmp(Hash, Rhs.Hash, sizeof Hash);
	}

	inline bool operator !=(const FIoChunkHash& Rhs) const
	{
		return !(*this == Rhs);
	}

private:
	uint8_t Hash[32];
};