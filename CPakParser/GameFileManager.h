#pragma once

#include "CoreTypes.h"

struct FPakEntryLocation // i dont like that i have to include this here but whatever
{
	static const int32_t Invalid = MIN_int32;
	static const int32_t MaxIndex = MAX_int32 - 1;

	FPakEntryLocation() : Index(Invalid)
	{
	}

	friend FArchive& operator<<(FArchive& Ar, FPakEntryLocation& Entry);

	friend size_t hash_value(const FPakEntryLocation& in)
	{
		return (in.Index * 0xdeece66d + 0xb);
	}

	__forceinline friend bool operator==(const FPakEntryLocation& entry1, const FPakEntryLocation& entry2)
	{
		return entry1.Index == entry2.Index;
	}

private:
	explicit FPakEntryLocation(int32_t InIndex) : Index(InIndex)
	{
	}

	int32_t Index;
};

typedef phmap::flat_hash_map<std::string, struct FPakEntryLocation> FPakDirectory;
typedef phmap::flat_hash_map<std::string, FPakDirectory> FDirectoryIndex;

class FGameFileManager
{
public:

	__forceinline static FGameFileManager& Get()
	{
		static FGameFileManager Inst;
		return Inst;
	}

	static void Debug()
	{
		auto& Files = Get().FileLibrary;

		for (auto i : Files)
		{
			for (auto x : i.second)
			{
				printf("%s%s\n", i.first.c_str(), x.first.c_str());
			}
		}
	}

	static FPakDirectory& Test(std::string dir)
	{
		if (!dir.ends_with('/'))
			dir += '/';

		return Get().FileLibrary[dir];
	}

	__forceinline static void AddFile(std::string& FileDir, std::string& FileName)
	{
		if (!Get().FileLibrary.contains(FileDir))
		{
			Get().FileLibrary.insert_or_assign(FileDir, FPakDirectory());
		}

		Get().FileLibrary[FileDir].insert_or_assign(FileName, FPakEntryLocation());
	}

	friend class FArchive& operator<<(class FArchive& Ar, FGameFileManager& Manager);

private:

	FGameFileManager() = default;

	FDirectoryIndex FileLibrary;
	std::mutex CriticalSection;
};