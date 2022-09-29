#pragma once

#include "CoreTypes.h"

struct FPakEntryLocation : public FFileEntryInfo
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

	bool IsInvalid() const
	{
		return Entry.PakIndex <= Invalid || MaxIndex < Entry.PakIndex;
	}

	bool IsOffsetIntoEncoded() const
	{
		return 0 <= Entry.PakIndex && Entry.PakIndex <= MaxIndex;
	}

	bool IsListIndex() const
	{
		return (-MaxIndex - 1) <= Entry.PakIndex && Entry.PakIndex <= -1;
	}

	int32_t  GetAsOffsetIntoEncoded() const
	{
		if (IsOffsetIntoEncoded())
		{
			return Entry.PakIndex;
		}
		
		return -1;
	}

	int32_t GetAsListIndex() const
	{
		if (IsListIndex())
		{
			return -(Entry.PakIndex + 1);
		}
		
		return -1;
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

class FGameFileManager // TODO: FDirectoryIterator
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
			printf("%s\n", i.first.c_str());
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

	/// <summary>
	/// </summary>
	/// <param name="Directory">Null terminated directory name with extension.</param>
	/// <param name="FileName">Null terminated file name with extension.</param>
	/// <returns></returns>
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

	/// <summary>
	/// Returns a list of file entries by directory.
	/// </summary>
	/// <param name="Directory">Null terminated directory string.</param>
	/// <returns></returns>
	__forceinline static FPakDirectory GetDirectory(std::string Directory)
	{
		return Get().FileLibrary[Directory];
	}

	static void SerializePakIndexes(FArchive& Ar, std::string& MountPoint, std::shared_ptr<class FPakFile> AssociatedPak);

private:

	FGameFileManager() = default;

	FDirectoryIndex FileLibrary;
	std::mutex CriticalSection;
};

static const std::string BaseMountPoint("../../../");

struct FGameFilePath
{
	FGameFilePath()
	{
	}

	FGameFilePath(const char* FilePath) : FGameFilePath(std::string(FilePath))
	{
	}

	FGameFilePath(const char* FileDirectory, const char* FileName) 
		: FGameFilePath(std::string(FileDirectory), std::string(FileName))
	{
	}

	FGameFilePath(std::string InFilePath)
	{
		auto idx = InFilePath.find_last_of('/');

		if (idx == std::string::npos)
			return;

		idx++;

		FileName = InFilePath.substr(idx);
		Directory = InFilePath.substr(0, idx);

		if (!Directory.starts_with(BaseMountPoint)) // TODO: a better less lazier way
			Directory = BaseMountPoint + Directory;

		if (Directory.back() != '\0')
			Directory += '\0';

		if (FileName.back() != '\0')
			FileName += '\0';
	}

	FGameFilePath(std::string InFileDirectory, std::string InFileName)
		: Directory(InFileDirectory), FileName(InFileName)
	{
		if (Directory.back() == '\0')
			Directory[Directory.size() - 1] = '/';
		else if (Directory.back() != '/')
			Directory += '/';

		if (FileName.back() != '\0')
			FileName += '\0';

		if (!Directory.starts_with(BaseMountPoint)) // TODO: a better less lazier way
			Directory = BaseMountPoint + Directory;

		Directory += '\0';
	}

	__forceinline bool IsValid()
	{
		return (!Directory.empty() && !FileName.empty());
	}

	__forceinline FFileEntryInfo GetEntryInfo()
	{
		return FGameFileManager::FindFile(Directory, FileName);
	}

private:

	/*
	* FPackagePath has to have null terminated directory and filename strings in order to grab
	* them from the directory index because serialized strings are null terminated.
	*/

	std::string Directory;
	std::string FileName;
};