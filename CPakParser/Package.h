#pragma once

#include "Loader.h"
#include "GameFileManager.h"

static const std::string BaseMountPoint("../../../");

struct FPackagePath
{
	FPackagePath()
	{
	}

	FPackagePath(std::string InPackagePath) 
	{
		auto idx = InPackagePath.find_last_of('/');

		if (idx == std::string::npos)
			return;

		idx++;

		FileName = InPackagePath.substr(idx);
		Directory = InPackagePath.substr(0, idx);

		if (!Directory.starts_with(BaseMountPoint)) // TODO: a better less lazier way
			Directory = BaseMountPoint + Directory;

		if (Directory.back() != '\0')
			Directory += '\0';

		if (FileName.back() != '\0')
			FileName += '\0';
	}

	FPackagePath(std::string InPackageDirectory, std::string InPackageName) 
		: Directory(InPackageDirectory), FileName(InPackageName)
	{
		if (Directory.back() == '\0')
			Directory[Directory.size() - 1] = '/';
		else if (Directory.back() != '/')
			Directory += '/';

		if (FileName.back() != '\0')
			FileName += '\0';

		if (!Directory.starts_with(BaseMountPoint)) // TODO: a better less lazier way
			Directory = BaseMountPoint + Directory;

		Directory += '\0';
	}

	__forceinline bool IsValid()
	{
		return (!Directory.empty() && !FileName.empty());
	}

	__forceinline FFileEntryInfo GetEntryInfo()
	{
		return FGameFileManager::FindFile(Directory, FileName);
	}

private:

	/*
	* FPackagePath has to have null terminated directory and filename strings in order to grab
	* them from the directory index because serialized strings are null terminated.
	*/

	std::string Directory; 
	std::string FileName;
};

class FLoader;

class UPackage
{
	FPackagePath Path;
	std::shared_ptr<FLoader> Linker;

	UPackage()
	{
	}

public:

	friend class FLoader;

	UPackage(FPackagePath PackagePath) : Path(PackagePath)
	{
	}

	__forceinline FPackagePath GetPath()
	{
		return Path;
	}

	__forceinline bool IsValid()
	{
		return Path.IsValid();
	}

	__forceinline bool HasLoader()
	{
		return !!Linker;
	}

	__forceinline std::shared_ptr<FLoader> GetLoader()
	{
		return Linker;
	}
};