#include "PakFileManager.h"
#include "IOStoreReader.h"

FPakFileManager::FPakFileManager() : bSigned(false)
{
}

bool FPakFileManager::RegisterEncryptionKey(FGuid InGuid, FAESKey InKey)
{
	if (FEncryptionKeyManager::HasKey(InGuid))
	{
		ReadStatus(ReadErrorCode::InvalidEncryptionKey, "Attempting to register a key with a GUID that has already been registered.");
		return false;
	}

	FEncryptionKeyManager::AddKey(InGuid, InKey);

	if (!bIsInitialized) return true; // if the pak manager hasn't been initialized yet, the key will get cached and used when the dataminer gets initialized

	if (!DeferredPaks.contains(InGuid)) return false;

	auto DynamicPakPath = DeferredPaks[InGuid];

	if (Mount(DynamicPakPath))
	{
		SCOPE_LOCK(CriticalSection);
		DeferredPaks.erase(InGuid);
	}
	else return false;

	return true;
}

FAESKey FPakFileManager::GetRegisteredPakEncryptionKey(const FGuid& InEncryptionKeyGuid)
{
	FAESKey Ret;

	if (!FEncryptionKeyManager::GetKey(InEncryptionKeyGuid, Ret))
	{
		//
	}

	return Ret;
}

bool FPakFileManager::Initialize(std::string InPaksFolderDir)
{
	if (bIsInitialized) return true;

	PaksFolderDir = InPaksFolderDir;

	auto GlobalUTocPath = std::filesystem::path(PaksFolderDir) /= "global.utoc";

	if (std::filesystem::exists(GlobalUTocPath))
	{
		ReadStatus(ReadErrorCode::Ok, "Mounting Global");

		IoFileBackend = std::make_shared<FFileIoStore>();
		PackageStoreBackend = std::make_shared<FFilePackageStoreBackend>();
		FPackageStore::Mount(PackageStoreBackend);

		IoFileBackend->Mount(GlobalUTocPath.string(), FGuid(), FAESKey());

		ReadStatus(ReadErrorCode::Ok, "Successfully mounted Global TOC");
	}

	this->MountAllPakFiles();

	return bIsInitialized = true;
}

bool FPakFileManager::Mount(std::filesystem::path InPakFilePath, bool bLoadIndex) //TODO: attempt to mount a toc even 
{
	auto PakFilename = InPakFilePath.filename().string();

	ReadStatus(ReadErrorCode::Ok, "Mounting PAK file: " + PakFilename);

	auto Pak = std::make_shared<FPakFile>(InPakFilePath, bSigned);
	Pak->Initialize(bLoadIndex);

	auto PakGuid = Pak->GetInfo().EncryptionKeyGuid;

	if (FEncryptionKeyManager::HasKey(PakGuid) || !PakGuid.IsValid())
	{
		FAESKey Key;
		FEncryptionKeyManager::GetKey(PakGuid, Key);

		auto TocPath = InPakFilePath.replace_extension(".utoc");

		if (std::filesystem::exists(TocPath))
		{
			auto Container = IoFileBackend->Mount(TocPath.string(), PakGuid, Key);

			if (Container.IsValid())
			{
				Pak->IoContainerHeader = std::make_unique<FIoContainerHeader>(Container);
			}
		}
	}
	else
	{
		SCOPE_LOCK(CriticalSection);

		if (!DeferredPaks.contains(PakGuid))
		{
			DeferredPaks.insert_or_assign(PakGuid, InPakFilePath);
		}

		ReadStatus(ReadErrorCode::Cancelled, "Encryption key could not be found for " + PakFilename + " so it will be deferred until it's registered");

		return false;
	}

	OnPakMounted(Pak);

	return true;
}

void FPakFileManager::MountAllPakFiles()
{
	std::vector<std::filesystem::path> PakFiles;
	for (auto& File : std::filesystem::directory_iterator(PaksFolderDir))
	{
		if (File.path().extension() == ".pak")
			PakFiles.push_back(File.path());
	}

	auto MountedPaks = GetMountedPaks();

	phmap::flat_hash_set<std::filesystem::path> MountedPakNames;

	for (auto Pak : MountedPaks)
	{
		MountedPakNames.insert(Pak->GetDiskPath());
	}

	// TODO: asynchronous mounting

	for (auto PakPath : PakFiles)
	{
		if (MountedPakNames.contains(PakPath))
			continue;

		if (!Mount(PakPath))
		{
			ReadStatus(ReadErrorCode::Cancelled, "Could not mount PAK file: " + PakPath.filename().string());
		}
	}

	ReadStatus(ReadErrorCode::Ok, "Mounted all available PAK files");
}