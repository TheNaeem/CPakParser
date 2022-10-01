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

void Dataminer::WithOodleDecompressor(const char* OodleDllPath)
{
	Oodle::LoadDLL(OodleDllPath);
}

FLocalization Dataminer::ReadLocRes(FGameFilePath FilePath)
{
	return ReadLocRes(FilePath.GetEntryInfo());
}

FLocalization Dataminer::ReadLocRes(FFileEntryInfo Entry)
{
	FLocalization Ret;

	auto Reader = FLoader::CreateFileReader(Entry);

	if (!Reader)
		return Ret;

	FLocalization LocRes;
	*Reader << LocRes;

	return Ret;
}

void Dataminer::Test(FGameFilePath Path)
{
	if (!Path.GetEntryInfo().IsValid())
		return;
}