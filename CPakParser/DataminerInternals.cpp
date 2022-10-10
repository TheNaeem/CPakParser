#include "Dataminer.h"

void Dataminer::OnPakMounted(TSharedPtr<FPakFile> Pak)
{
	SCOPE_LOCK(MountCritSection);

	Pak->SetIsMounted(true);

	Log<Success>("Sucessfully mounted PAK file: " + Pak->GetFilename());

	MountedFiles.push_back(Pak);
}

bool Dataminer::MountPak(std::filesystem::path InPakFilePath, bool bLoadIndex) //TODO: attempt to mount a toc even 
{
	auto PakFilename = InPakFilePath.filename().string();

	Log<Success>("Mounting PAK file: " + PakFilename);

	auto Pak = std::make_shared<FPakFile>(InPakFilePath, Context);
	Pak->Initialize(bLoadIndex);

	auto PakGuid = Pak->GetInfo().EncryptionKeyGuid;

	if (Context->EncryptionKeyManager.HasKey(PakGuid) || !PakGuid.IsValid())
	{
		FAESKey Key;
		Context->EncryptionKeyManager.GetKey(PakGuid, Key);

		auto TocPath = InPakFilePath.replace_extension(".utoc");

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

		Log<Warning>("Encryption key could not be found for " + PakFilename + " so it will be deferred until it's registered");

		return false;
	}

	OnPakMounted(Pak);

	return true;
}

TSharedPtr<FIoStoreReader> Dataminer::MountToc(std::string InTocPath, FGuid EncryptionKeyGuid, FAESKey EncryptionKey)
{
	Log<Success>("Mounting TOC: " + InTocPath);

	auto Reader = std::make_shared<FIoStoreReader>(InTocPath.c_str(), PartitionIndex);
	auto Toc = Reader->Initialize(Context, true);

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

		if (Path.extension() != ".pak")
		{
			continue;
		}

		MountedPakNames.insert(Pak->GetDiskPath());
	}

	// TODO: asynchronous mounting

	for (auto PakPath : PakFiles)
	{
		if (MountedPakNames.contains(PakPath))
			continue;

		if (!MountPak(PakPath))
		{
			Log<Warning>("Could not mount PAK file: " + PakPath.filename().string());
		}
	}

	Log<Success>("Mounted all available PAK files");
}