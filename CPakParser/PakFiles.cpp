#include "PakFiles.h"

#include "IOStoreReader.h"
#include <filesystem>

FPakPlatformFile::FPakPlatformFile(std::string InPaksFolderDir)
	: PaksFolderDir(InPaksFolderDir)
	, bSigned(false)
{
}

FPackageStore::FPackageStore()
{
}

void FPackageStore::Mount(std::shared_ptr<FFilePackageStoreBackend> Backend)
{
	Get().Backends.push_back(Backend);
}

bool FPakPlatformFile::Initialize()
{
	ExcludedNonPakExtensions.insert("uasset");
	ExcludedNonPakExtensions.insert("umap");
	ExcludedNonPakExtensions.insert("ubulk");
	ExcludedNonPakExtensions.insert("uexp");
	ExcludedNonPakExtensions.insert("uptnl");
	ExcludedNonPakExtensions.insert("ushaderbytecode");

	auto GlobalUTocPath = std::filesystem::path(PaksFolderDir) /= "global.utoc";
	const bool bShouldMountGlobal = std::filesystem::exists(GlobalUTocPath);

	if (bShouldMountGlobal)
	{
		IoFileBackend = std::make_shared<FFileIoStore>();
		PackageStoreBackend = std::make_shared<FFilePackageStoreBackend>();
		FPackageStore::Mount(PackageStoreBackend);

		IoFileBackend->Mount(GlobalUTocPath.string().c_str(), 0, FGuid(), FAESKey());
	}

	this->MountAllPakFiles();

	return true;
}

bool FPakPlatformFile::Mount(std::string InPakFilename, bool bLoadIndex = true)
{

}

int FPakPlatformFile::MountAllPakFiles()
{
	int NumPakFilesMounted = 0;

	std::vector<std::string> PakFiles;
	for (auto& File : std::filesystem::directory_iterator(PaksFolderDir))
	{
		if (File.path().extension() == ".pak")
			PakFiles.push_back(File.path().filename().string());
	}

	auto MountedPaks = GetMountedPaks();

	std::set<std::string> MountedPakNames;

	for (auto Pak : MountedPaks)
	{
		MountedPakNames.insert(Pak.PakFile->GetFilename());
	}

	for (auto PakFileName : PakFiles)
	{
		if (MountedPakNames.contains(PakFileName))
			continue;

		if (Mount(PakFileName))
		{
			NumPakFilesMounted++;
		}
	}

	return NumPakFilesMounted;
}
