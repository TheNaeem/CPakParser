#include "Dataminer.h"

#include "PakFiles.h"

static FPakFileManager PakPlatformFile;

void Dataminer::Initialize(const char* PaksFolderDir)
{
	PakPlatformFile.Initialize(PaksFolderDir);
}

bool Dataminer::SubmitKey(const char* AesKeyString, const char* GuidString)
{
	return PakPlatformFile.RegisterEncryptionKey(GuidString ? FGuid(GuidString) : FGuid(), FAESKey(AesKeyString));
}