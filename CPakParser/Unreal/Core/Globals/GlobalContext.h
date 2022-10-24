#pragma once

#include "Core/UObject.h"
#include "GameFileManager.h"
#include "Files/IOStore/Toc/GlobalTocData.h"
#include "Misc/Encryption/EncryptionKeyManager.h"

class GContext
{
public:

	FEncryptionKeyManager EncryptionKeyManager;
	FGameFileManager FilesManager;
	struct FGlobalTocData GlobalToc;
	TMap<std::string, UObjectPtr> ObjectArray;
};