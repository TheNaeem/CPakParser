#include "Loader.h"

#include "FileReader.h"
#include "MemoryReader.h"

FLoader::FLoader(UPackage& InPackage) 
	: Package(InPackage), bHasSerializedPackageFileSummary(false)
{
}

FLoader::~FLoader()
{
	if (Reader)
		delete Reader;
}

FArchive* FLoader::CreateFileReader(FGameFilePath Path, bool bMemoryPreload)
{
	// TODO: async loader https://github.com/EpicGames/UnrealEngine/blob/ue5-main/Engine/Source/Runtime/CoreUObject/Private/UObject/LinkerLoad.cpp#L1064

	auto EntryInfo = Path.GetEntryInfo();

	if (!EntryInfo.IsValid())
		return nullptr;

	auto File = EntryInfo.GetAssociatedFile()->CreateEntryHandle(EntryInfo);

	if (bMemoryPreload)
	{
		auto Size = File->TotalSize();
		auto Buffer = malloc(Size);
		File->Serialize(Buffer, Size);

		delete File;

		return new FMemoryReader(static_cast<uint8_t*>(Buffer), Size, true);
	}

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
