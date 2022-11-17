#include <string>

import CPakParser.Files.GameFilePath;

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