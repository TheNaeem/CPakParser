#include "Dataminer.h"

#include "PakFiles.h"

void Dataminer::Initialize(const char* PaksFolderDir)
{
	FPakPlatformFile PakPlatformFile(PaksFolderDir);
	PakPlatformFile.Initialize(nullptr);
}