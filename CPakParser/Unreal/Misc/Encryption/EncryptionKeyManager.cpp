#include "EncryptionKeyManager.h"

#include "../Multithreading/Lock.h"

void FEncryptionKeyManager::AddKey(const FGuid& InGuid, const FAESKey InKey)
{
	SCOPE_LOCK(CriticalSection);

	if (!Keys.contains(InGuid))
	{
		Keys.insert_or_assign(InGuid, InKey);
	}
}

bool FEncryptionKeyManager::GetKey(const FGuid& InGuid, FAESKey& OutKey)
{
	SCOPE_LOCK(CriticalSection);

	if (!Keys.contains(InGuid)) return false;

	OutKey = Keys[InGuid];
	return true;
}

bool const FEncryptionKeyManager::HasKey(const FGuid& InGuid)
{
	return Keys.contains(InGuid);
}

const TMap<FGuid, FAESKey>& FEncryptionKeyManager::GetKeys()
{
	return Keys;
}