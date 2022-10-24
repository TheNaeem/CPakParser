#pragma once

#include "AES.h"
#include "../Guid.h"
#include "../Hashing/Map.h"

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