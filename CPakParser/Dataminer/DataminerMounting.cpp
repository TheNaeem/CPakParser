#include "Dataminer.h"

#include "Core/Globals/GlobalContext.h"
#include "Files/IOStore/Toc/IoStoreToc.h"
#include "Files/IOStore/Misc/IoStoreReader.h"
#include "Misc/Multithreading/Lock.h"
#include <filesystem>

import CPakParser.Logging;
import CPakParser.Serialization.FArchive;
import CPakParser.Paks.PakFile;

void Dataminer::OnPakMounted(TSharedPtr<FPakFile> Pak)
{
	SCOPE_LOCK(MountCritSection);

	Pak->SetIsMounted(true);

	Log("Sucessfully mounted PAK file %s", Pak->GetDiskPath().c_str());

	MountedFiles.push_back(Pak);
}

bool Dataminer::MountPak(std::string InPakFilePath, bool bLoadIndex) //TODO: attempt to mount a toc even 
{
	Log("Mounting PAK file %s", InPakFilePath.c_str());

	auto Pak = std::make_shared<FPakFile>(InPakFilePath, Context);
	Pak->Initialize(bLoadIndex);

	auto PakGuid = Pak->GetInfo().EncryptionKeyGuid;

	if (Context->EncryptionKeyManager.HasKey(PakGuid) || !PakGuid.IsValid())
	{
		FAESKey Key;
		Context->EncryptionKeyManager.GetKey(PakGuid, Key);

		auto TocPath = std::filesystem::path(InPakFilePath).replace_extension(".utoc");

		if (std::filesystem::exists(TocPath))
		{
			if (!MountToc(TocPath.string(), PakGuid, Key))
			{
				return false;
			}
		}
	}
	else
	{
		SCOPE_LOCK(MountCritSection);

		if (!UnmountedPaks.contains(PakGuid))
		{
			UnmountedPaks.insert_or_assign(PakGuid, InPakFilePath);
		}

		LogWarn("Encryption key could not be found for %s so it will be deferred until it's registered", InPakFilePath.c_str());

		return false;
	}

	OnPakMounted(Pak);

	return true;
}

TSharedPtr<FIoStoreReader> Dataminer::MountToc(std::string InTocPath, FGuid EncryptionKeyGuid, FAESKey EncryptionKey)
{
	Log("Mounting TOC %s", InTocPath.c_str());

	auto Toc = std::make_shared<FIoStoreToc>(InTocPath);
	auto Reader = std::make_shared<FIoStoreReader>(Toc, PartitionIndex);
	Reader->Initialize(Context, true);

	if (!Toc)
		return nullptr;

	if (Reader->IsEncrypted() && Context->EncryptionKeyManager.HasKey(EncryptionKeyGuid))
	{
		Reader->SetEncryptionKey(EncryptionKey);
	}

	Reader->ReadContainerHeader();

	MountedFiles.push_back(Toc);

	return Reader;
}

void Dataminer::MountAllPakFiles()
{
	std::vector<std::filesystem::path> PakFiles;
	for (auto& File : std::filesystem::directory_iterator(PaksDirectory))
	{
		if (File.path().extension() == ".pak")
			PakFiles.push_back(File.path());
	}

	auto MountedPaks = GetMountedFiles();

	phmap::flat_hash_set<std::filesystem::path> MountedPakNames;

	for (auto Pak : MountedPaks)
	{
		auto Path = Pak->GetDiskPath();

		if (!Path.ends_with(".pak"))
		{
			continue;
		}

		MountedPakNames.insert(Path);
	}

	// TODO: asynchronous mounting

	for (auto PakPath : PakFiles)
	{
		if (MountedPakNames.contains(PakPath))
			continue;

		if (!MountPak(PakPath.string()))
		{
			LogWarn("Could not mount PAK file %s", PakPath.filename().string().c_str());
		}
	}

	Log("Mounted all available PAK files");
}