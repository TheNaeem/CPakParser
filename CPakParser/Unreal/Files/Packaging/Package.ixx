module;

#include "Core/Defines.h"

export module CPakParser.Package;

export import CPakParser.Core.UObject;
import CPakParser.Files.GameFilePath;

export class UPackage : public UObject // TODO: change how package works
{
	FGameFilePath Path;
	TSharedPtr<class FLoader> Linker;

protected:

	UPackage()
	{
	}

	TSharedPtr<class GContext> Context;

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