#pragma once

#include "PakFiles.h"

class FPakFileManager
{
	std::mutex CriticalSection;
	std::string PaksFolderDir;
	std::vector<std::shared_ptr<FPakFile>> PakFiles;
	bool bSigned;
	bool bIsInitialized = false;
	std::string IniFileExtension;
	std::string GameUserSettingsIniFilename;
	std::shared_ptr<class FFileIoStore> IoFileBackend;
	std::shared_ptr<FFilePackageStoreBackend> PackageStoreBackend;
	phmap::flat_hash_map<FGuid, std::filesystem::path> DeferredPaks;

public:

	FPakFileManager();

	bool Initialize(std::string InPaksFolderDir);
	bool Mount(std::filesystem::path InPakFilename, bool bLoadIndex = true);
	void MountAllPakFiles();

	bool RegisterEncryptionKey(FGuid InGuid, FAESKey InKey);
	static FAESKey GetRegisteredPakEncryptionKey(const FGuid& InEncryptionKeyGuid);

	std::vector<std::shared_ptr<FPakFile>> GetMountedPaks()
	{
		return PakFiles;
	}

	void OnPakMounted(std::shared_ptr<FPakFile> Pak)
	{
		SCOPE_LOCK(CriticalSection);

		Pak->bIsMounted = true;

		ReadStatus(ReadErrorCode::Ok, "Sucessfully mounted PAK file: " + Pak->GetFilename());

		PakFiles.push_back(Pak);
	}
};