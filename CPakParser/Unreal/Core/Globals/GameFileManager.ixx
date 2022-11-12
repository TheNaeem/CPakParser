module;

#include "../Defines.h"
#include "Misc/Hashing/Map.h"

export module GameFileManager;

import FileEntry;
import FArchiveBase;

import <string>;
import <vector>;

#if HASH_DIRECTORY_INDEX

typedef std::vector<std::pair<std::string, FFileEntryInfo>> FGameFileCollection;
typedef TMap<uint32_t, FFileEntryInfo> FPakDirectory;
typedef phmap::parallel_flat_hash_map<uint32_t, FPakDirectory> FDirectoryIndex;

#else

typedef std::vector<std::pair<std::string, FFileEntryInfo>> FGameFileCollection;
typedef TMap<std::string, FFileEntryInfo> FPakDirectory;
typedef phmap::parallel_flat_hash_map<std::string, FPakDirectory> FDirectoryIndex;

#endif

export struct FGameFilePath
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

export class FGameFileManager // TODO: FDirectoryIterator
{
public:

	void Reserve(size_t Count);

	/// <summary>
	/// </summary>
	/// <param name="Directory">Null terminated directory name with extension.</param>
	/// <param name="FileName">Null terminated file name with extension.</param>
	/// <returns></returns>
	FFileEntryInfo FindFile(std::string& Directory, std::string& FileName);
	FFileEntryInfo FindFile(FGameFilePath& Path);

	/// <summary>
	/// Returns a list of file entries by directory.
	/// </summary>
	/// <param name="Directory">Null terminated directory string.</param>
	/// <returns></returns>
	FPakDirectory GetDirectory(std::string Directory);
	FGameFileCollection GetFilesByExtension(std::string Ext);
	FDirectoryIndex GetFiles();

	void SerializePakIndexes(FArchive& Ar, std::string& MountPoint, TSharedPtr<IDiskFile> AssociatedPak);

#if HASH_DIRECTORY_INDEX
	void AddFile(std::string& FileDir, std::string& FileName, FFileEntryInfo EntryInfo);
#else
	__forceinline void AddFile(std::string& FileDir, std::string& FileName, FFileEntryInfo EntryInfo)
	{
		if (!FileLibrary.contains(FileDir))
		{
			FileLibrary.insert_or_assign(FileDir, FPakDirectory());
		}

		FileLibrary[FileDir].insert_or_assign(FileName, EntryInfo);
	}
#endif

private:

	FDirectoryIndex FileLibrary;
};
