#pragma once

#include "GameFileManager.h"
#include "GlobalTocData.h"

class GContext
{
public:

	FEncryptionKeyManager EncryptionKeyManager;
	FGameFileManager FilesManager;
	FGlobalTocData GlobalToc;
};