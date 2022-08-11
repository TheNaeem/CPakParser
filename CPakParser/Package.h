#pragma once

#include "Loader.h"
#include "GameFileManager.h"

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
	}

	FPackagePath(std::string InPackageDirectory, std::string InPackageName) 
		: Directory(InPackageDirectory), FileName(InPackageName)
	{
	}

	__forceinline bool IsValid()
	{
		return (!Directory.empty() && !FileName.empty());
	}

	__forceinline std::string ToString() { return Directory + FileName; }

	__forceinline FFileEntryInfo GetEntry()
	{
		return FGameFileManager::FindFile(Directory, FileName);
	}

private:

	std::string Directory;
	std::string FileName;
};

class FLoader;

class UPackage
{
	FPackagePath Path;
	FFileEntryInfo Entry;
	std::shared_ptr<FLoader> Linker;

	UPackage()
	{
	}

public:

	friend class FLoader;

	UPackage(FPackagePath PackagePath) 
		: Path(PackagePath), Entry(Path.GetEntry())
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

	__forceinline std::string GetOwningFileName()
	{
		return Entry.GetDiskFilePath().string();
	}
};