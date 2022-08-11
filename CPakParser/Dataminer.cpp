#include "Dataminer.h"

#include "PakFileManager.h"
#include "Loader.h"

static FPakFileManager PakPlatformFile;

void Dataminer::Initialize(const char* PaksFolderDir)
{
	PakPlatformFile.Initialize(PaksFolderDir);
}

bool Dataminer::SubmitKey(const char* AesKeyString, const char* GuidString)
{
	return PakPlatformFile.RegisterEncryptionKey(GuidString ? FGuid(GuidString) : FGuid(), FAESKey(AesKeyString));
}

bool Dataminer::Test(const char* FileDirectory, const char* FileName)
{
	auto PackagePath = FPackagePath(FileDirectory, FileName);
	auto Package = UPackage(PackagePath);
	auto Loader = FLoader::FromPackage(Package);

	Loader->LoadAllObjects();

	return true;
}