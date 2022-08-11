#pragma once

#include "CoreTypes.h"

struct FPakEntryLocation : public FFileEntryInfo // i dont like that i have to include this here but whatever
{
	static const int32_t Invalid = MIN_int32;
	static const int32_t MaxIndex = MAX_int32 - 1;

	FPakEntryLocation() : FFileEntryInfo(Invalid)
	{
	}

	friend size_t hash_value(const FPakEntryLocation& in)
	{
		return (in.Entry.PakIndex * 0xdeece66d + 0xb);
	}

	__forceinline friend bool operator==(const FPakEntryLocation& entry1, const FPakEntryLocation& entry2)
	{
		return entry1.Entry.PakIndex == entry2.Entry.PakIndex;
	}


private:

	explicit FPakEntryLocation(int32_t InIndex)
	{
		Entry.PakIndex = InIndex;
	}
};

struct FIoStoreTocChunkInfo : public FFileEntryInfo
{
	uint64_t Offset;
	uint64_t Size;
};

typedef phmap::flat_hash_map<std::string, FFileEntryInfo> FPakDirectory;
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

	__forceinline static void AddFile(std::string& FileDir, std::string& FileName, FFileEntryInfo EntryInfo)
	{
		if (!Get().FileLibrary.contains(FileDir))
		{
			Get().FileLibrary.insert_or_assign(FileDir, FPakDirectory());
		}

		Get().FileLibrary[FileDir].insert_or_assign(FileName, EntryInfo);
	}

	__forceinline static FFileEntryInfo FindFile(std::string& Directory, std::string& FileName)
	{
		auto& Lib = Get().FileLibrary;

		if (!Lib.contains(Directory))
			return FFileEntryInfo();

		auto Dir = Lib[Directory];

		if (!Dir.contains(FileName))
			return FFileEntryInfo();

		return Dir[FileName];
	}

	static void SerializePakIndexes(FArchive& Ar, std::shared_ptr<class FPakFile> AssociatedPak);

private:

	FGameFileManager() = default;

	FDirectoryIndex FileLibrary;
	std::mutex CriticalSection;
};