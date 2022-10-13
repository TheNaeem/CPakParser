#pragma once

#include "GameFileManager.h"
#include "GlobalTocData.h"
#include "UObject.h"

class GContext
{
public:

	FEncryptionKeyManager EncryptionKeyManager;
	FGameFileManager FilesManager;
	FGlobalTocData GlobalToc;
	phmap::flat_hash_map<std::string, UObjectPtr> ObjectArray;
};