#include <string>

import CPakParser.Files.GameFilePath;
import CPakParser.Files.FileEntry;

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
	auto FileNameIndex = InFilePath.find_last_of('/');

	if (FileNameIndex == std::string::npos)
		return;

	FileNameIndex++;
	
	Directory = InFilePath.substr(0, FileNameIndex);

	auto ExportIndex = InFilePath.find_last_of('.');

	if (ExportIndex != std::string::npos)
	{
		FileName = InFilePath.substr(FileNameIndex, ExportIndex - FileNameIndex);
		ExportName = InFilePath.substr(ExportIndex + 1);
	}
	else
	{
		FileName = InFilePath.substr(FileNameIndex);
		ExportName = FileName;
	}

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

FGameFilePath FGameFilePath::WithExtension(std::string Extension)
{
	return FGameFilePath(Directory, FileName + Extension);
}

