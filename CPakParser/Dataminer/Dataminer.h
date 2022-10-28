#pragma once

#include "Core/Defines.h"
#include "Core/Globals/GameFileManager.h"
#include "Misc/Guid.h"
#include "Misc/Encryption/AES.h"
#include "Misc/Hashing/Map.h"
#include "Files/SerializableFile.h"
#include <future>

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

	template <typename T>
	TSharedPtr<T> SerializeFile(FGameFilePath FilePath)
	{
		static_assert(std::is_base_of<ISerializableFile, T>::value, "File to serialize must inherit from ISerializableFile");

		auto Ret = std::make_shared<T>();

		SerializeFileInternal(Ret);

		return Ret;
	}
};