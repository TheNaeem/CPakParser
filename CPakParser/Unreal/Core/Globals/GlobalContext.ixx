export module GlobalContext;

#include "Files/IOStore/Toc/GlobalTocData.h"
#include "Misc/Encryption/EncryptionKeyManager.h"

import UObjectCore;
import PackageFileVersion;
import GameFileManager;

class GContext
{
public:

	FEncryptionKeyManager EncryptionKeyManager;
	FGameFileManager FilesManager;
	struct FGlobalTocData GlobalToc;
	TMap<std::string, UObjectPtr> ObjectArray;
	FPackageFileVersion GPackageFileUEVersion{ VER_UE4_AUTOMATIC_VERSION, EUnrealEngineObjectUE5Version::AUTOMATIC_VERSION };
};