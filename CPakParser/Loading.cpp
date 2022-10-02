#include "Loader.h"

#include "FileReader.h"
#include "MemoryReader.h"

FLoader::FLoader(UPackage& InPackage) 
	: Package(InPackage), bHasSerializedPackageFileSummary(false), Reader(nullptr)
{
}

FLoader::~FLoader()
{
}

FUniqueAr FLoader::CreateFileReader(FGameFilePath Path, bool bMemoryPreload)
{
	return CreateFileReader(Path.GetEntryInfo(), bMemoryPreload);
}

FUniqueAr FLoader::CreateFileReader(FFileEntryInfo Entry, bool bMemoryPreload)
{
	if (!Entry.IsValid())
		return nullptr;

	auto File = Entry.GetAssociatedFile()->CreateEntryArchive(Entry);

	return File;
}

__forceinline bool FLoader::IsValid()
{
	return Package.IsValid();
}

std::shared_ptr<FLoader> FLoader::FromPackage(UPackage& Package)
{
	if (Package.HasLoader()) return Package.GetLoader();

	struct LoaderImpl : FLoader // i know its kind of iffy but its like this for a reason
	{
		LoaderImpl(UPackage& Pkg) : FLoader(Pkg)
		{
		}
	};

	return std::make_shared<LoaderImpl>(Package);
}

void FLoader::SerializePackageSummary()
{
	if (bHasSerializedPackageFileSummary)
		return;

	*Reader << Summary;
}

void FLoader::LoadAllObjects()
{
	if (!Reader)
		Reader = CreateFileReader(Package.GetPath());

	if (!Reader)
		return;

	SerializePackageSummary();
}
