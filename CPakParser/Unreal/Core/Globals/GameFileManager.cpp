

#include "Files/Paks/PakEntryLocation.h"
#include "Serialization/Archives.h"

import GameFileManager;

#if HASH_DIRECTORY_INDEX
static constexpr auto Hash(const char* s, int off = 0) -> unsigned int
{
	return !s[off] ? 5381 : (Hash(s, off + 1) * 33) ^ s[off];
}

#define QUICK_STR_HASH(str) Hash(str.c_str())
#endif

// TODO: hash directory index

static const std::string BaseMountPoint("../../../");

FGameFilePath::FGameFilePath()
{
}

FGameFilePath::FGameFilePath(const char* FilePath) : FGameFilePath(std::string(FilePath))
{
}

FGameFilePath::FGameFilePath(const char* FileDirectory, const char* FileName)
	: FGameFilePath(std::string(FileDirectory), std::string(FileName))
{
}

FGameFilePath::FGameFilePath(std::string InFilePath)
{
	auto idx = InFilePath.find_last_of('/');

	if (idx == std::string::npos)
		return;

	idx++;

	FileName = InFilePath.substr(idx);
	Directory = InFilePath.substr(0, idx);

	if (!Directory.starts_with(BaseMountPoint)) // TODO: a better less lazier way
		Directory = BaseMountPoint + Directory;
}

FGameFilePath::FGameFilePath(std::string InFileDirectory, std::string InFileName)
	: Directory(InFileDirectory), FileName(InFileName)
{
	if (Directory.back() != '/')
		Directory += '/';

	if (!Directory.starts_with(BaseMountPoint)) // TODO: a better less lazier way
		Directory = BaseMountPoint + Directory;
}

bool FGameFilePath::IsValid()
{
	return (!Directory.empty() && !FileName.empty());
}

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

FPakDirectory FGameFileManager::GetDirectory(std::string Directory)
{
#if HASH_DIRECTORY_INDEX
	return FileLibrary[QUICK_STR_HASH(Directory)];
#else
	return FileLibrary[Directory];
#endif
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