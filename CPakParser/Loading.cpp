#include "Loader.h"
#include "FileReader.h"

FLoader::FLoader(UPackage& InPackage) : Package(InPackage)
{
	this->CreateLoader();
}

void FLoader::CreateLoader()
{
	// TODO: async loader https://github.com/EpicGames/UnrealEngine/blob/ue5-main/Engine/Source/Runtime/CoreUObject/Private/UObject/LinkerLoad.cpp#L1064

	this->Reader = FFileReader(Package.GetOwningFileName().c_str());
}

void FLoader::LoadAllObjects()
{

}

__forceinline bool FLoader::IsValid()
{
	return Package.IsValid();
}

std::shared_ptr<FLoader> FLoader::FromPackage(UPackage& Package)
{
	if (Package.HasLoader()) return Package.GetLoader();

	return std::make_shared<FLoader>(FLoader(Package));
}