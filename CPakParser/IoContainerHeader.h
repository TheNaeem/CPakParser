#pragma once

#include "CoreTypes.h"
#include "IoContainer.h"

struct FIoContainerHeaderPackageRedirect
{
	FPackageId SourcePackageId;
	FPackageId TargetPackageId;
	FMappedName SourcePackageName;

	friend FArchive& operator<<(FArchive& Ar, FIoContainerHeaderPackageRedirect& PackageRedirect);
};

struct FIoContainerHeaderLocalizedPackage
{
	FPackageId SourcePackageId;
	FMappedName SourcePackageName;

	friend FArchive& operator<<(FArchive& Ar, FIoContainerHeaderLocalizedPackage& LocalizedPackage);
};

struct FIoContainerHeader
{
	enum
	{
		Signature = 0x496f436e
	};

	enum class Version : uint32_t
	{
		Initial = 0,
		LocalizedPackages = 1,
		OptionalSegmentPackages = 2,

		LatestPlusOne,
		Latest = LatestPlusOne - 1
	};

	FIoContainerId ContainerId;
	std::vector<FPackageId> PackageIds;
	std::vector<uint8_t> StoreEntries;
	std::vector<FPackageId> OptionalSegmentPackageIds;
	std::vector<uint8_t> OptionalSegmentStoreEntries;
	std::vector<std::string> RedirectsNameMap;
	std::vector<FIoContainerHeaderLocalizedPackage> LocalizedPackages;
	std::vector<FIoContainerHeaderPackageRedirect> PackageRedirects;

	friend FArchive& operator<<(FArchive& Ar, FIoContainerHeader& ContainerHeader);

	__forceinline bool IsValid()
	{
		return ContainerId.IsValid();
	}
};