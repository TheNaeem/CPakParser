export module CPakParser.Files.GameFilePath;

export import <string>;
import <optional>;

export struct FGameFilePath
{
public:

	friend class FGameFileManager;

	FGameFilePath();
	FGameFilePath(const char* FilePath);
	FGameFilePath(const char* FileDirectory, const char* FileName);
	FGameFilePath(std::string InFilePath);
	FGameFilePath(std::string InFileDirectory, std::string InFileName);

	bool IsValid();
	FGameFilePath WithExtension(std::string Extension);

	/*
	* FPackagePath has to have null terminated directory and filename strings in order to grab
	* them from the directory index because serialized strings are null terminated.
	*/

	std::string Directory;
	std::string FileName;
	std::string ExportName;
};