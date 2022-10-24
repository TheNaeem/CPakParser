#include "ShaHash.h"

#include "Serialization/Archives.h"

FSHAHash::FSHAHash()
{
	memset(Hash, 0, sizeof(Hash));
}

bool operator==(const FSHAHash& X, const FSHAHash& Y)
{
	return memcmp(&X.Hash, &Y.Hash, sizeof(X.Hash)) == 0;
}

bool operator!=(const FSHAHash& X, const FSHAHash& Y)
{
	return memcmp(&X.Hash, &Y.Hash, sizeof(X.Hash)) != 0;
}

bool operator<(const FSHAHash& X, const FSHAHash& Y)
{
	return memcmp(&X.Hash, &Y.Hash, sizeof(X.Hash)) < 0;
}

FArchive& operator<<(FArchive& Ar, FSHAHash& G)
{
	Ar.Serialize(&G.Hash, sizeof(G.Hash));
	return Ar;
}

uint32_t hash_value(const FSHAHash& InKey)
{
	return *reinterpret_cast<const uint32_t*>(InKey.Hash);
}