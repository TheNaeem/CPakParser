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

	int32_t GetAsOffsetIntoEncoded() const
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

typedef std::vector<std::pair<std::string, FFileEntryInfo>> FGameFileCollection;
typedef phmap::flat_hash_map<std::string, FFileEntryInfo> FPakDirectory;
typedef phmap::flat_hash_map<std::string, FPakDirectory> FDirectoryIndex;

class FGameFileManager // TODO: FDirectoryIterator
{
public:

	void Reserve(size_t Count);
	void AddFile(std::string& FileDir, std::string& FileName, FFileEntryInfo EntryInfo);

	/// <summary>
	/// </summary>
	/// <param name="Directory">Null terminated directory name with extension.</param>
	/// <param name="FileName">Null terminated file name with extension.</param>
	/// <returns></returns>
	FFileEntryInfo FindFile(std::string& Directory, std::string& FileName);
	FFileEntryInfo FindFile(struct FGameFilePath& Path);

	/// <summary>
	/// Returns a list of file entries by directory.
	/// </summary>
	/// <param name="Directory">Null terminated directory string.</param>
	/// <returns></returns>
	__forceinline FPakDirectory GetDirectory(std::string Directory);
	__forceinline FGameFileCollection GetFilesByExtension(std::string Ext);
	__forceinline FDirectoryIndex GetFiles();

	void SerializePakIndexes(FArchive& Ar, std::string& MountPoint, TSharedPtr<class FPakFile> AssociatedPak);

private:

	FDirectoryIndex FileLibrary;
	std::mutex CriticalSection;
};

struct FGameFilePath
{
	friend class FGameFileManager;

	FGameFilePath();
	FGameFilePath(const char* FilePath);
	FGameFilePath(const char* FileDirectory, const char* FileName);
	FGameFilePath(std::string InFilePath);
	FGameFilePath(std::string InFileDirectory, std::string InFileName);

	__forceinline bool IsValid();

private:

	/*
	* FPackagePath has to have null terminated directory and filename strings in order to grab
	* them from the directory index because serialized strings are null terminated.
	*/

	std::string Directory;
	std::string FileName;
};