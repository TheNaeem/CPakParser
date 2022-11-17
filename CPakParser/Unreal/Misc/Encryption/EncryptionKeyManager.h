#pragma once

#include "Misc/Hashing/Map.h"

import CPakParser.Encryption.AES;
import CPakParser.Misc.FGuid;

class FEncryptionKeyManager
{
public:

	void AddKey(const FGuid& InGuid, const FAESKey InKey);

	bool GetKey(const FGuid& InGuid, FAESKey& OutKey);

	bool const HasKey(const FGuid& InGuid);

	const TMap<FGuid, FAESKey>& GetKeys();

private:

	TMap<FGuid, FAESKey> Keys;
	std::mutex CriticalSection;
};
