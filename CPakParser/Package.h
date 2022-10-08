#pragma once

#include "Loader.h"
#include "GameFileManager.h"

class FLoader;

class UPackage
{
	FGameFilePath Path;
	TSharedPtr<FLoader> Linker;

protected:
	
	UPackage()
	{
	}

	std::string Name;

public:

	friend class FLoader;

	UPackage(FGameFilePath PackagePath) : Path(PackagePath)
	{
	}

	__forceinline std::string GetName()
	{
		return Name;
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

	__forceinline TSharedPtr<FLoader> GetLoader()
	{
		return Linker;
	}
};