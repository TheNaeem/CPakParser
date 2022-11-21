#include "GameFileManager.h"

#include <string>

import CPakParser.Paks.PakEntryLocation;

#if HASH_DIRECTORY_INDEX
static constexpr auto Hash(const char* s, int off = 0) -> unsigned int
{
	return !s[off] ? 5381 : (Hash(s, off + 1) * 33) ^ s[off];
}

#define QUICK_STR_HASH(str) Hash(str.c_str())
#endif


void FGameFileManager::Reserve(size_t Count)
{
	FileLibrary.reserve(Count);
}

FDirectoryIndex FGameFileManager::GetFiles()
{
	return FileLibrary;
}

FFileEntryInfo FGameFileManager::FindFile(FGameFilePath& Path)
{
	return FindFile(Path.Directory, Path.FileName);
}

FPakDirectory FGameFileManager::GetDirectory(std::string& Directory)
{
#if HASH_DIRECTORY_INDEX
	return FileLibrary[QUICK_STR_HASH(Directory)];
#else
	return FileLibrary[Directory];
#endif
}

bool FGameFileManager::DirectoryExists(std::string& Dir)
{
#if HASH_DIRECTORY_INDEX
	return FileLibrary.contains(QUICK_STR_HASH(Dir));
#else
	return FileLibrary.contains(Dir);
#endif
}

std::optional<FPakDirectory> FGameFileManager::TryGetDirectory(std::string& Dir)
{
#if HASH_DIRECTORY_INDEX
	auto Directory = QUICK_STR_HASH(Dir);
#else
	auto& Directory = Dir;
#endif

	if (FileLibrary.contains(Directory))
	{
		return FileLibrary[Directory];
	}

	return std::nullopt;
}

FGameFileCollection FGameFileManager::GetFilesByExtension(std::string Ext)
{
	FGameFileCollection Ret;

#if !HASH_DIRECTORY_INDEX
	if (!Ext.ends_with('\0'))
		Ext.push_back('\0');

	for (auto Dir : FileLibrary)
	{
		for (auto File : Dir.second)
		{
			if (File.first.ends_with(Ext))
			{
				Ret.push_back(File);
			}
		}
	}
#endif

	return Ret;
}

FFileEntryInfo FGameFileManager::FindFile(std::string& InDirectory, std::string& InFileName)
{
#if HASH_DIRECTORY_INDEX
	auto Directory = QUICK_STR_HASH(InDirectory);
	auto FileName = QUICK_STR_HASH(InFileName);
#else
	auto& Directory = InDirectory;
	auto& FileName = InFileName;
#endif

	if (!FileLibrary.contains(Directory))
		return FFileEntryInfo();

	auto Dir = FileLibrary[Directory];

	if (!Dir.contains(FileName))
		return FFileEntryInfo();

	return Dir[FileName];

}

void FGameFileManager::SerializePakIndexes(FArchive& Ar, std::string& MountPoint, TSharedPtr<IDiskFile> AssociatedPak)
{
	int32_t NewNumElements = 0;
	Ar << NewNumElements;

	if (!NewNumElements) return;

	FileLibrary.reserve(NewNumElements);

	for (size_t i = 0; i < NewNumElements; i++)
	{
		/*
		* FYI: I don't like this either.
		* But it's important for assigning the FFileEntryInfo as an FPakEntryLocation,
		* as well as for assigning the shared pak pointer.
		* I would of course prefer reducing this to much less lines and making it cleaner by just directly serializing the FPakDirectory.
		* But it is what it is.
		* Will revisit this another time, so it is a TODO, but for now this will do. At least it doesn't affect speed.
		*/

		std::string DirectoryName;
		FPakDirectory DirIdx;

		Ar << DirectoryName;

		int32_t DirIdxNum = 0;
		Ar << DirIdxNum;

		if (!DirIdxNum) continue;

		DirectoryName = MountPoint + DirectoryName;
		DirIdx.reserve(DirIdxNum);

		DirectoryName.pop_back(); // remove the null terminator

		for (size_t i = 0; i < DirIdxNum; i++)
		{
			std::string Name;
			FPakEntryLocation Entry;

			Ar << Name << Entry;

			Name.pop_back(); // remove the null terminator

			Entry.SetOwningFile(AssociatedPak.get());

#if HASH_DIRECTORY_INDEX
			DirIdx.insert_or_assign(QUICK_STR_HASH(Name), Entry);
#else
			DirIdx.insert_or_assign(Name, Entry);
#endif
		}

#if HASH_DIRECTORY_INDEX
		auto DirHash = QUICK_STR_HASH(DirectoryName);

		if (!FileLibrary.contains(DirHash))
		{
			FileLibrary.insert_or_assign(DirHash, DirIdx);
			continue;
		}

		FileLibrary[DirHash].merge(DirIdx);
#else
		if (!FileLibrary.contains(DirectoryName))
		{
			FileLibrary.insert_or_assign(DirectoryName, DirIdx);
			continue;
		}

		FileLibrary[DirectoryName].merge(DirIdx);
#endif
	}
}

#if HASH_DIRECTORY_INDEX
void FGameFileManager::AddFile(std::string& InFileDir, std::string& InFileName, FFileEntryInfo EntryInfo)
{
	auto FileDir = QUICK_STR_HASH(InFileDir);

	if (!FileLibrary.contains(FileDir))
	{
		FileLibrary.insert_or_assign(FileDir, FPakDirectory());
	}

	auto FileName = QUICK_STR_HASH(InFileName);

	FileLibrary[FileDir].insert_or_assign(FileName, EntryInfo);
}
#endif