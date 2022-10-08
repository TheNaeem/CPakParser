#pragma once

#include "PakFiles.h"

class FPakFileManager // TODO: make this into Dataminer
{
	std::mutex CriticalSection;
	std::string PaksFolderDir;
	std::vector<TSharedPtr<FPakFile>> PakFiles;
	bool bSigned;
	bool bIsInitialized = false;
	std::string IniFileExtension;
	std::string GameUserSettingsIniFilename;
	TSharedPtr<class FFileIoStore> IoFileBackend;
	TSharedPtr<FFilePackageStoreBackend> PackageStoreBackend;
	phmap::flat_hash_map<FGuid, std::filesystem::path> DeferredPaks;

public:

	FPakFileManager();

	bool Initialize(std::string InPaksFolderDir);
	bool Mount(std::filesystem::path InPakFilename, bool bLoadIndex = true);
	void MountAllPakFiles();

	bool RegisterEncryptionKey(FGuid InGuid, FAESKey InKey);
	static FAESKey GetRegisteredPakEncryptionKey(const FGuid& InEncryptionKeyGuid);

	std::vector<TSharedPtr<FPakFile>> GetMountedPaks()
	{
		return PakFiles;
	}

	void OnPakMounted(TSharedPtr<FPakFile> Pak)
	{
		SCOPE_LOCK(CriticalSection);

		Pak->bIsMounted = true;

		Log<Success>("Sucessfully mounted PAK file: " + Pak->GetFilename());

		PakFiles.push_back(Pak);
	}
};