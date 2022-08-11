#pragma once

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
	static bool Test(const char* FileDirectory, const char* FileName);
};