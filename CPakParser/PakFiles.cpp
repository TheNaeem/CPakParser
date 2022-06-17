#include "PakFiles.h"

#include <filesystem>
#include "IODispatcher.h"

FPakPlatformFile::FPakPlatformFile(std::string InPaksFolderDir)
	: PaksFolderDir(InPaksFolderDir)
	, LowerLevel(NULL)
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

bool FPakPlatformFile::Initialize(FPakPlatformFile* Inner)
{
	LowerLevel = Inner;

	ExcludedNonPakExtensions.insert("uasset");
	ExcludedNonPakExtensions.insert("umap");
	ExcludedNonPakExtensions.insert("ubulk");
	ExcludedNonPakExtensions.insert("uexp");
	ExcludedNonPakExtensions.insert("uptnl");
	ExcludedNonPakExtensions.insert("ushaderbytecode");

	static std::string StartupPaksWildcard = "*.pak";

	auto GlobalUTocPath = std::filesystem::path(PaksFolderDir) /= "global.utoc";
	const bool bShouldMountGlobal = std::filesystem::exists(GlobalUTocPath);

	if (bShouldMountGlobal)
	{
		IoDispatcherFileBackend = std::make_shared<FFileIoStore>();
		FIoDispatcher::Mount(IoDispatcherFileBackend);
		PackageStoreBackend = std::make_shared<FFilePackageStoreBackend>();
		FPackageStore::Mount(PackageStoreBackend);

	    IoDispatcherFileBackend->Mount(GlobalUTocPath.string().c_str(), 0, FGuid(), FAESKey());
	}

	return true;
}