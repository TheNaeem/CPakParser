export module GlobalContext;

import UObject;
#include "GameFileManager.h"
#include "Files/IOStore/Toc/GlobalTocData.h"
#include "Misc/Encryption/EncryptionKeyManager.h"
#include "Misc/Versioning/PackageFileVersion.h"

class GContext
{
public:

	FEncryptionKeyManager EncryptionKeyManager;
	FGameFileManager FilesManager;
	struct FGlobalTocData GlobalToc;
	TMap<std::string, UObjectPtr> ObjectArray;
	FPackageFileVersion GPackageFileUEVersion{ VER_UE4_AUTOMATIC_VERSION, EUnrealEngineObjectUE5Version::AUTOMATIC_VERSION };
};