#pragma once

#include "Core/Defines.h"
#include "Core/Globals/GameFileManager.h"
#include <future>

import CPakParser.Package;
import CPakParser.Files.GameFilePath;
import CPakParser.Files.SerializableFile;
import CPakParser.Misc.FGuid;
import CPakParser.Encryption.AES; 
import CPakParser.Files.ExportState;

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

	/* INITIALIZATION */

	bool Initialize();
	bool SubmitKey(const char* AesKeyString, const char* GuidString = nullptr);
	bool LoadTypeMappings(std::string UsmapFilePath);
	std::future<bool> LoadTypeMappingsAsync(std::string UsmapFilePath);

	/* GETTERS AND SETTERS */

	std::vector<TSharedPtr<IDiskFile>> GetMountedFiles();
	TMap<FGuid, std::string> GetUnmountedPaks();
	FDirectoryIndex Files();
	TMap<std::string, UObjectPtr> GetObjectArray();
	FPakDirectory GetDirectory(std::string InDirectory);
	std::optional<FPakDirectory> TryGetDirectory(std::string InDirectory);
	void SetVersionUE5(int Version);
	void SetVersionUE4(int Version);

	/* LOADING */

	template <typename T>
	TSharedPtr<T> SerializeFile(FGameFilePath FilePath)
	{
		static_assert(std::is_base_of<ISerializableFile, T>::value, "File to serialize must inherit from ISerializableFile");

		auto Ret = std::make_shared<T>();

		SerializeFileInternal(Ret);

		return Ret;
	}

	UPackagePtr LoadPackage(FGameFilePath Path);
	UPackagePtr LoadPackage(FFileEntryInfo& Entry);
	UPackagePtr LoadPackage(FGameFilePath Path, FExportState& State);
	UPackagePtr LoadPackage(FFileEntryInfo& Entry, FExportState& State);

	template <typename T = UObject>
	TObjectPtr<T> LoadObject(FGameFilePath Path)
	{
		static_assert(std::is_base_of<UObject, T>::value, "Type passed into LoadObject must be a UObject type");

		FExportState State;
		State.TargetObject = std::make_shared<T>();
		State.TargetObjectName = Path.ExportName;
		State.LoadTargetOnly = true;

		auto Package = LoadPackage(Path, State);

		if (!Package)
			return nullptr;

		return Package->GetExportByName(Path.ExportName).As<T>();
	}

	std::optional<UPackagePtr> TryLoadPackage(FGameFilePath Path)
	{
		auto Ret = LoadPackage(Path);
		return Ret ? std::optional<UPackagePtr>(Ret) : std::nullopt;
	}

	template <typename T = UObject>
	std::optional<TObjectPtr<T>> TryLoadObject(FGameFilePath Path)
	{
		static_assert(std::is_base_of<UObject, T>::value, "Type passed into TryLoadObject must be a UObject type");

		auto Ret = LoadObject<T>(Path);
		return Ret ? std::optional<TObjectPtr<T>>(Ret) : std::nullopt;
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
#define UE5_2 1008
#define UE5_3 1009