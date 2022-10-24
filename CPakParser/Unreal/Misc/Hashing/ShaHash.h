#pragma once

class FSHAHash
{
public:

	unsigned __int8 Hash[20];

	FSHAHash();

	friend bool operator==(const FSHAHash& X, const FSHAHash& Y);
	friend bool operator!=(const FSHAHash& X, const FSHAHash& Y);
	friend bool operator<(const FSHAHash& X, const FSHAHash& Y);
	friend class FArchive& operator<<(FArchive& Ar, FSHAHash& G);

	friend unsigned __int32 hash_value(const FSHAHash& InKey);
};