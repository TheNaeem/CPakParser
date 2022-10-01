#pragma once

#include "GameFileManager.h"
#include "Localization.h"

class Dataminer
{
	Dataminer()
	{
	}

public:

	Dataminer& operator=(const Dataminer&) = delete;
	Dataminer(const Dataminer&) = delete; 

	static Dataminer& Get()
	{
		static Dataminer Instance;
		return Instance;
	}

	static void Initialize(const char* PaksFolderDir);
	static bool SubmitKey(const char* AesKeyString, const char* GuidString = nullptr);
	static void WithOodleDecompressor(const char* OodleDllPath);
	static FLocalization ReadLocRes(FGameFilePath FilePath);
	static FLocalization ReadLocRes(FFileEntryInfo Entry);
	static void Test(FGameFilePath Path);
};