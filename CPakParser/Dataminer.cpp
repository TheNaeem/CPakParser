#include "Dataminer.h"

#include "Loader.h"
#include "Oodle.h"
#include "Localization.h"
#include <future>

void Dataminer::Options::WithLogging(bool bEnableLogging)
{
	LOGGING_ENABLED = bEnableLogging;
}

void Dataminer::Options::WithOodleDecompressor(const char* OodleDllPath)
{
	Oodle::LoadDLL(OodleDllPath);
}

Dataminer::Dataminer(const char* PaksFolderDir) : 
	PaksDirectory(PaksFolderDir),
	Context(std::make_shared<GContext>())
{
}

std::vector<TSharedPtr<IDiskFile>> Dataminer::GetMountedFiles()
{
	return MountedFiles;
}

phmap::flat_hash_map<FGuid, std::filesystem::path> Dataminer::GetUnmountedPaks()
{
	return UnmountedPaks;
}

bool Dataminer::Initialize()
{
	if (bIsInitialized) return true;

	if (!std::filesystem::is_directory(PaksDirectory))
	{
		Log<Error>("Invalid paks directory.");
		return false;
	}

	auto GlobalUTocPath = std::filesystem::path(PaksDirectory) /= "global.utoc";

	std::future<void> GlobalMountTask;

	if (std::filesystem::exists(GlobalUTocPath))
	{
		GlobalMountTask = std::async(std::launch::async, [this, &GlobalUTocPath]()
			{
				Log<Info>("Mounting global.utoc");

				auto GlobalReader = MountToc(GlobalUTocPath.string(), FGuid(), FAESKey());

				if (!GlobalReader)
					return;

				Context->GlobalToc.Serialize(GlobalReader);

				Log<Success>("Successfully mounted Global TOC");
			});
	}

	this->MountAllPakFiles();

	GlobalMountTask.wait();

	return bIsInitialized = true;
}

bool Dataminer::SubmitKey(const char* AesKeyString, const char* GuidString)
{
	auto Guid = GuidString ? FGuid(GuidString) : FGuid();
	auto Key = FAESKey(AesKeyString);

	if (Context->EncryptionKeyManager.HasKey(Guid))
	{
		Log<Warning>("Attempting to register a key with a GUID that has already been registered.");
		return false;
	}

	Context->EncryptionKeyManager.AddKey(Guid, Key);

	if (!bIsInitialized) return true; // if the pak manager hasn't been initialized yet, the key will get cached and used when the dataminer gets initialized

	if (!UnmountedPaks.contains(Guid)) return false;

	auto DynamicPakPath = UnmountedPaks[Guid];

	if (MountPak(DynamicPakPath))
	{
		SCOPE_LOCK(MountCritSection);
		UnmountedPaks.erase(Guid);

		return true;
	}

	return false;
}

FLocalization Dataminer::ReadLocRes(FGameFilePath FilePath)
{
	return ReadLocRes(Context->FilesManager.FindFile(FilePath));
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
	auto Entry = Context->FilesManager.FindFile(Path);

	if (!Entry.IsValid())
		return;

	auto Reader = FLoader::CreateFileReader(Entry);

	Entry.GetAssociatedFile()->DoWork(Reader, Context);
}