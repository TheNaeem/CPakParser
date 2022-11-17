export module CPakParser.Hashing.ShaHash;

import CPakParser.Serialization.FArchive;
import <cstring>;

export class FSHAHash
{
public:

	unsigned __int8 Hash[20]{ 0 };

	FSHAHash()
	{
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

	friend FArchive& operator<<(FArchive& Ar, FSHAHash& G)
	{
		Ar.Serialize(&G.Hash, sizeof(G.Hash));
		return Ar;
	}

	friend unsigned __int32 hash_value(const FSHAHash& InKey)
	{
		return *reinterpret_cast<const uint32_t*>(InKey.Hash);
	}
};