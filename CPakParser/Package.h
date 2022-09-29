#pragma once

#include "Loader.h"
#include "GameFileManager.h"

class FLoader;

class UPackage
{
	FGameFilePath Path;
	std::shared_ptr<FLoader> Linker;

	UPackage()
	{
	}

public:

	friend class FLoader;

	UPackage(FGameFilePath PackagePath) : Path(PackagePath)
	{
	}

	__forceinline FGameFilePath GetPath()
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