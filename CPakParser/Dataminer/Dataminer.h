#pragma once

#include "Core/Defines.h"
#include "Core/Globals/GameFileManager.h"
#include "Misc/Guid.h"
#include "Misc/Encryption/AES.h"
#include "Misc/Hashing/Map.h"

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

	TSharedPtr<class GContext> Context;
	std::mutex MountCritSection;
	std::atomic_int32_t PartitionIndex = 0;
	bool bIsInitialized = false;
	std::vector<TSharedPtr<class IDiskFile>> MountedFiles;
	TMap<FGuid, std::string> UnmountedPaks;

private:

	TSharedPtr<class FIoStoreReader> MountToc(std::string InTocPath, FGuid EncryptionKeyGuid, FAESKey EncryptionKey);
	bool MountPak(std::string InPakFilePath, bool bLoadIndex = true);
	void MountAllPakFiles();
	void OnPakMounted(TSharedPtr<class FPakFile> Pak);

public:

	const std::string PaksDirectory;

	Dataminer(const char* PaksFolderDir);

	bool Initialize();
	bool SubmitKey(const char* AesKeyString, const char* GuidString = nullptr);
	struct FLocalization ReadLocRes(struct FGameFilePath FilePath);
	struct FLocalization ReadLocRes(struct FFileEntryInfo Entry);
	std::vector<TSharedPtr<IDiskFile>> GetMountedFiles();
	TMap<FGuid, std::string> GetUnmountedPaks();
	void Test(FGameFilePath Path);
	FDirectoryIndex Files();
};