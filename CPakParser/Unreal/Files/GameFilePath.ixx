export module CPakParser.Files.GameFilePath;

export import <string>;

export struct FGameFilePath
{
	friend class FGameFileManager;

	FGameFilePath();
	FGameFilePath(const char* FilePath);
	FGameFilePath(const char* FileDirectory, const char* FileName);
	FGameFilePath(std::string InFilePath);
	FGameFilePath(std::string InFileDirectory, std::string InFileName);

	bool IsValid();

private:

	/*
	* FPackagePath has to have null terminated directory and filename strings in order to grab
	* them from the directory index because serialized strings are null terminated.
	*/

	std::string Directory;
	std::string FileName;
};