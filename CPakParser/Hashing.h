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