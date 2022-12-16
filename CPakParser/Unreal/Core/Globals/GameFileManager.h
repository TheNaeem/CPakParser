#pragma once

#include "../Defines.h"
#include "Misc/Hashing/Map.h"
#include <string>;
#include <vector>;

import CPakParser.Files.FileEntry;
import CPakParser.Serialization.FArchive;
import CPakParser.Files.GameFilePath;

#define HASH_DIRECTORY_INDEX 1

#if HASH_DIRECTORY_INDEX

typedef std::vector<std::pair<std::string, FFileEntryInfo>> FGameFileCollection;
typedef TMap<uint32_t, FFileEntryInfo> FPakDirectory;
typedef parallel_flat_hash_map<uint32_t, FPakDirectory> FDirectoryIndex;

#else

typedef std::vector<std::pair<std::string, FFileEntryInfo>> FGameFileCollection;
typedef TMap<std::string, FFileEntryInfo> FPakDirectory;
typedef parallel_flat_hash_map<std::string, FPakDirectory> FDirectoryIndex;

#endif

class FGameFileManager // TODO: FDirectoryIterator, FGameFileViewer
{
public:

	void Reserve(size_t Count);

	FFileEntryInfo FindFile(std::string& Directory, std::string& FileName);
	FFileEntryInfo FindFile(FGameFilePath& Path);
	FPakDirectory GetDirectory(std::string& Directory);
	FGameFileCollection GetFilesByExtension(std::string Ext);
	FDirectoryIndex GetFiles();
	bool DirectoryExists(std::string& Dir);
	std::optional<FPakDirectory> TryGetDirectory(std::string& Dir);

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