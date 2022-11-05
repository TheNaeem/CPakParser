#include "Dataminer.h"

#include "Core/Globals/GlobalContext.h"
#include "Misc/Multithreading/Lock.h"
#include "Logger.h"
#include <filesystem>
#include <future>

Dataminer::Dataminer(const char* PaksFolderDir) :
	PaksDirectory(PaksFolderDir),
	Context(std::make_shared<GContext>())
{
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

void Dataminer::SetVersionUE5(int Version)
{
	Context->GPackageFileUEVersion = FPackageFileVersion(522, Version);
}

void Dataminer::SetVersionUE4(int Version)
{
	Context->GPackageFileUEVersion = FPackageFileVersion::CreateUE4Version(Version);
}