#pragma once

#include "Core/Defines.h"
#include "Files/FileEntry.h"
#include "Misc/Hashing/Map.h"

typedef std::vector<std::pair<std::string, FFileEntryInfo>> FGameFileCollection;
typedef TMap<std::string, FFileEntryInfo> FPakDirectory;
typedef TMap<std::string, FPakDirectory> FDirectoryIndex;

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
	FPakDirectory GetDirectory(std::string Directory);
	FGameFileCollection GetFilesByExtension(std::string Ext);
	FDirectoryIndex GetFiles();

	void SerializePakIndexes(FArchive& Ar, std::string& MountPoint, TSharedPtr<class IDiskFile> AssociatedPak);

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