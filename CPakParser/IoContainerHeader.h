#pragma once

#include "CoreTypes.h"
#include "Hashing.h"
#include <span>

template<typename T>
class TFilePackageStoreEntryCArrayView
{
	const uint32_t ArrayNum = 0;
	const uint32_t OffsetToDataFromThis = 0;

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
	TFilePackageStoreEntryCArrayView<FSHAHash> ShaderMapHashes;
};

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

	phmap::flat_hash_map<FPackageId, FFilePackageStoreEntry*> PackageStore;
	class FIoContainerId ContainerId;
	std::vector<FPackageId> PackageIds;
	std::span<FFilePackageStoreEntry> StoreEntries;
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