module;

#include "Misc/Multithreading/Lock.h"
#include "Misc/Hashing/Map.h"

export module EncryptionKeyManager;

export import AES;
export import Guid;

export class FEncryptionKeyManager
{
public:

	void AddKey(const FGuid& InGuid, const FAESKey InKey)
	{
		SCOPE_LOCK(CriticalSection);

		if (!Keys.contains(InGuid))
		{
			Keys.insert_or_assign(InGuid, InKey);
		}
	}

	bool GetKey(const FGuid& InGuid, FAESKey& OutKey)
	{
		SCOPE_LOCK(CriticalSection);

		if (!Keys.contains(InGuid)) return false;

		OutKey = Keys[InGuid];
		return true;
	}

	bool const HasKey(const FGuid& InGuid)
	{
		return Keys.contains(InGuid);
	}

	const TMap<FGuid, FAESKey>& GetKeys()
	{
		return Keys;
	}

private:

	TMap<FGuid, FAESKey> Keys;
	std::mutex CriticalSection;
};