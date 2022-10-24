#include "GameFileManager.h"

#include "Files/Paks/PakEntryLocation.h"
#include "Serialization/Archives.h"

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

void FGameFileManager::AddFile(std::string& FileDir, std::string& FileName, FFileEntryInfo EntryInfo)
{
	if (!FileLibrary.contains(FileDir))
	{
		FileLibrary.insert_or_assign(FileDir, FPakDirectory());
	}

	FileLibrary[FileDir].insert_or_assign(FileName, EntryInfo);
}

FFileEntryInfo FGameFileManager::FindFile(std::string& Directory, std::string& FileName)
{
	if (!FileLibrary.contains(Directory))
		return FFileEntryInfo();

	auto Dir = FileLibrary[Directory];

	if (!Dir.contains(FileName))
		return FFileEntryInfo();

	return Dir[FileName];
}

FFileEntryInfo FGameFileManager::FindFile(FGameFilePath& Path)
{
	return FindFile(Path.Directory, Path.FileName);
}

FPakDirectory FGameFileManager::GetDirectory(std::string Directory)
{
	return FileLibrary[Directory];
}

FGameFileCollection FGameFileManager::GetFilesByExtension(std::string Ext)
{
	FGameFileCollection Ret;

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

	return Ret;
}

FDirectoryIndex FGameFileManager::GetFiles()
{
	return FileLibrary;
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

			Entry.SetOwningFile(AssociatedPak);

			DirIdx.insert_or_assign(Name, Entry);
		}

		if (!FileLibrary.contains(DirectoryName))
		{
			FileLibrary.insert_or_assign(DirectoryName, DirIdx);
			continue;
		}

		FileLibrary[DirectoryName].merge(DirIdx);
	}
}