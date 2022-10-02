#pragma once

#include "CoreTypes.h"

class FIoContainerId
{
public:
	inline FIoContainerId() : Id(InvalidId)
	{
	}

	inline FIoContainerId(const FIoContainerId& Other) = default;
	inline FIoContainerId(FIoContainerId&& Other) = default;
	inline FIoContainerId& operator=(const FIoContainerId& Other) = default;

	uint64_t Value() const
	{
		return Id;
	}

	inline bool IsValid() const
	{
		return Id != InvalidId;
	}

	inline bool operator<(FIoContainerId Other) const
	{
		return Id < Other.Id;
	}

	inline bool operator==(FIoContainerId Other) const
	{
		return Id == Other.Id;
	}

	inline bool operator!=(FIoContainerId Other) const
	{
		return Id != Other.Id;
	}

	inline friend uint32_t GetTypeHash(const FIoContainerId& In)
	{
		return uint32_t(In.Id);
	}

	friend FArchive& operator<<(FArchive& Ar, FIoContainerId& ContainerId);

private:
	inline explicit FIoContainerId(const uint64_t InId)
		: Id(InId) { }

	static constexpr uint64_t InvalidId = uint64_t(-1);

	uint64_t Id;
};

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

	class FIoContainerId ContainerId;
	std::vector<FPackageId> PackageIds;
	std::vector<uint8_t> StoreEntries;
	std::vector<FPackageId> OptionalSegmentPackageIds;
	std::vector<uint8_t> OptionalSegmentStoreEntries;
	std::vector<std::string> RedirectsNameMap;
	std::vector<struct FIoContainerHeaderLocalizedPackage> LocalizedPackages;
	std::vector<struct FIoContainerHeaderPackageRedirect> PackageRedirects;

	friend FArchive& operator<<(FArchive& Ar, FIoContainerHeader& ContainerHeader);

	__forceinline bool IsValid()
	{
		return ContainerId.IsValid();
	}
};