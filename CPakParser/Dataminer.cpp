#include "Dataminer.h"

#include "PakFileManager.h"
#include "Loader.h"
#include "Oodle.h"
#include "Localization.h"

static FPakFileManager PakPlatformFile;

void Dataminer::Initialize(const char* PaksFolderDir)
{
	PakPlatformFile.Initialize(PaksFolderDir);
}

bool Dataminer::SubmitKey(const char* AesKeyString, const char* GuidString)
{
	return PakPlatformFile.RegisterEncryptionKey(GuidString ? FGuid(GuidString) : FGuid(), FAESKey(AesKeyString));
}

void Dataminer::WithOodleCompressor(const char* OodleDllPath)
{
	Oodle::LoadDLL(OodleDllPath);
}

bool Dataminer::Test(FGameFilePath FilePath)
{
	/*auto Package = UPackage(FilePath);
	auto Loader = FLoader::FromPackage(Package);

	Loader->LoadAllObjects();*/

	auto Reader = FLoader::CreateFileReader(FilePath);
	
	if (!Reader)
		return false;

	FLocalization LocRes;
	*Reader << LocRes;

	delete Reader;

	return true;
}