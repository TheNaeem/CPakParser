#include "Dataminer.h"

#include "Core/Globals/GlobalContext.h"
#include "Misc/Compression/Oodle.h"
#include "Logger.h"

// getters and setters

void Dataminer::Options::WithLogging(bool bEnableLogging)
{
	LOGGING_ENABLED = bEnableLogging;
}

void Dataminer::Options::WithOodleDecompressor(const char* OodleDllPath)
{
	Oodle::LoadDLL(OodleDllPath);
}

std::vector<TSharedPtr<IDiskFile>> Dataminer::GetMountedFiles()
{
	return MountedFiles;
}

TMap<FGuid, std::string> Dataminer::GetUnmountedPaks()
{
	return UnmountedPaks;
}

FDirectoryIndex Dataminer::Files()
{
	return Context->FilesManager.GetFiles();
}

TMap<std::string, UObjectPtr> Dataminer::GetObjectArray()
{
	return Context->ObjectArray;
}