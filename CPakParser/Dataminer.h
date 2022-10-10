#pragma once

#include "GameFileManager.h"
#include "Localization.h"
#include "IoStoreReader.h"
#include "PakFiles.h"
#include "GlobalTocData.h"

class Dataminer
{
public:

	class Options
	{
	public:

		static void WithLogging(bool bEnableLogging);
		static void WithOodleDecompressor(const char* OodleDllPath);
	};

private:

	TSharedPtr<GContext> Context;
	std::mutex MountCritSection;
	std::atomic_int32_t PartitionIndex = 0;
	bool bIsInitialized = false;
	std::vector<TSharedPtr<IDiskFile>> MountedFiles;
	phmap::flat_hash_map<FGuid, std::filesystem::path> UnmountedPaks;

private:

	TSharedPtr<FIoStoreReader> MountToc(std::string InTocPath, FGuid EncryptionKeyGuid, FAESKey EncryptionKey);
	bool MountPak(std::filesystem::path InPakFilePath, bool bLoadIndex = true);
	void MountAllPakFiles();
	void OnPakMounted(TSharedPtr<FPakFile> Pak);

public:

	const std::string PaksDirectory;

	Dataminer(const char* PaksFolderDir);

	bool Initialize();
	bool SubmitKey(const char* AesKeyString, const char* GuidString = nullptr);
	FLocalization ReadLocRes(struct FGameFilePath FilePath);
	FLocalization ReadLocRes(FFileEntryInfo Entry);
	std::vector<TSharedPtr<IDiskFile>> GetMountedFiles();
	phmap::flat_hash_map<FGuid, std::filesystem::path> GetUnmountedPaks();
	void Test(FGameFilePath Path);
};