#include "GameFileManager.h"

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