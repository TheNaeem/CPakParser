#pragma once

#include "Misc/Hashing/Map.h"

import CPakParser.Package.Id;
import CPakParser.Hashing.ShaHash;
import CPakParser.Names.MappedName;
import CPakParser.IOStore.ContainerId;

template<typename T>
class TFilePackageStoreEntryCArrayView
{
	uint32_t ArrayNum = 0;
	uint32_t OffsetToDataFromThis = 0;

public:
	__forceinline uint32_t Num() const { return ArrayNum; }

	__forceinline const T* Data() const { return (T*)((char*)this + OffsetToDataFromThis); }
	__forceinline T* Data() { return (T*)((char*)this + OffsetToDataFromThis); }

	__forceinline const T* begin() const { return Data(); }
	__forceinline T* begin() { return Data(); }

	__forceinline const T* end() const { return Data() + ArrayNum; }
	__forceinline T* end() { return Data() + ArrayNum; }

	__forceinline const T& operator[](uint32_t Index) const { return Data()[Index]; }
	__forceinline T& operator[](uint32_t Index) { return Data()[Index]; }
};

struct FFilePackageStoreEntry
{
	int32_t ExportCount;
	int32_t ExportBundleCount;
	TFilePackageStoreEntryCArrayView<FPackageId> ImportedPackages;
	TFilePackageStoreEntryCArrayView<class FSHAHash> ShaderMapHashes;
};

struct FIoContainerHeaderPackageRedirect
{
	FPackageId SourcePackageId;
	FPackageId TargetPackageId;
	FMappedName SourcePackageName;
};

struct FIoContainerHeaderLocalizedPackage
{
	FPackageId SourcePackageId;
	FMappedName SourcePackageName;
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

	TMap<FPackageId, FFilePackageStoreEntry*> PackageStore;
	FIoContainerId ContainerId;
	std::vector<FPackageId> PackageIds;
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

private:

	std::vector<uint8_t> StoreEntriesData;
	std::vector<uint8_t> OptionalSegmentStoreEntriesData;
};