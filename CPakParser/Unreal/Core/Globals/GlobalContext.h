#pragma once

#include "GameFileManager.h"
#include "Misc/Encryption/EncryptionKeyManager.h"
#include "Files/IOStore/Toc/GlobalTocData.h"

import CPakParser.Core.UObject;
import CPakParser.Versions.PackageFileVersion;

class GContext
{
public:

	FEncryptionKeyManager EncryptionKeyManager;
	FGameFileManager FilesManager;
	FGlobalTocData GlobalToc;
	TMap<std::string, UObjectPtr> ObjectArray;
	FPackageFileVersion GPackageFileUEVersion{ VER_UE4_AUTOMATIC_VERSION, EUnrealEngineObjectUE5Version::AUTOMATIC_VERSION };
};