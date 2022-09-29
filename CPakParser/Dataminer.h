#pragma once

#include "GameFileManager.h"

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
	static void WithOodleCompressor(const char* OodleDllPath);
	static bool Test(FGameFilePath FilePath);
};