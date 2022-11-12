#pragma once

#include "Core/Defines.h"
#include "Core/Globals/GameFileManager.h"
#include "Misc/Guid.h"
#include "Misc/Encryption/AES.h"
#include "Misc/Hashing/Map.h"
#include "Misc/Versioning/PackageFileVersion.h"
#include "Files/SerializableFile.h"
#include <future>

import UObjectCore;

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

	void SerializeFileInternal(FGameFilePath& FilePath, TSharedPtr<ISerializableFile> OutFile);

public:

	const std::string PaksDirectory;

	Dataminer(const char* PaksFolderDir);

	bool Initialize();
	bool SubmitKey(const char* AesKeyString, const char* GuidString = nullptr);
	bool LoadTypeMappings(std::string UsmapFilePath);
	std::future<bool> LoadTypeMappingsAsync(std::string UsmapFilePath);

	std::vector<TSharedPtr<IDiskFile>> GetMountedFiles();
	TMap<FGuid, std::string> GetUnmountedPaks();
	void Test(FGameFilePath Path);
	FDirectoryIndex Files();
	TMap<std::string, UObjectPtr> GetObjectArray();
	void SetVersionUE5(int Version);
	void SetVersionUE4(int Version);

	template <typename T>
	TSharedPtr<T> SerializeFile(FGameFilePath FilePath)
	{
		static_assert(std::is_base_of<ISerializableFile, T>::value, "File to serialize must inherit from ISerializableFile");

		auto Ret = std::make_shared<T>();

		SerializeFileInternal(Ret);

		return Ret;
	}
};

// Shoutout to CUE4Parse for these values
#define UE4_1 342
#define UE4_2 352
#define UE4_3 363
#define UE4_4 382
#define UE4_5 385
#define UE4_6 401
#define UE4_7 413
#define UE4_8 434
#define UE4_9 451
#define UE4_10 482
#define UE4_11 482
#define UE4_12 498
#define UE4_13 504
#define UE4_14 505
#define UE4_15 508
#define UE4_16 510
#define UE4_17 513
#define UE4_18 513
#define UE4_19 514
#define UE4_20 516
#define UE4_21 516
#define UE4_22 517
#define UE4_23 517
#define UE4_24 517
#define UE4_25 518
#define UE4_26 518
#define UE4_27 522

#define UE5_0 1004
#define UE5_1 1006