#pragma once

#include "Core/UObject.h"
#include "Core/Names/NameMap.h"
#include "Misc/Versioning/PackageFileVersion.h"
#include "Misc/Versioning/CustomVersion.h"
#include "../PackageId.h"
#include "../PackageObjectIndex.h"
#include <vector>

enum class EExportFilterFlags : uint8_t
{
	None,
	NotForClient,
	NotForServer
};

struct FExportBundleEntry
{
	enum EExportCommandType
	{
		ExportCommandType_Create,
		ExportCommandType_Serialize,
		ExportCommandType_Count
	};

	uint32_t LocalExportIndex;
	uint32_t CommandType;
};

struct FExportBundleHeader
{
	uint64_t SerialOffset;
	uint32_t FirstEntryIndex;
	uint32_t EntryCount;
};

struct FExportMapEntry
{
	uint64_t CookedSerialOffset = 0;
	uint64_t CookedSerialSize = 0;
	FMappedName ObjectName;
	FPackageObjectIndex OuterIndex;
	FPackageObjectIndex ClassIndex;
	FPackageObjectIndex SuperIndex;
	FPackageObjectIndex TemplateIndex;
	uint64_t PublicExportHash;
	UObject::Flags ObjectFlags = UObject::Flags::RF_NoFlags;
	EExportFilterFlags FilterFlags = EExportFilterFlags::None;
	uint8_t Pad[3] = {};
};

enum class EZenPackageVersion : uint32_t
{
	Initial,

	LatestPlusOne,
	Latest = LatestPlusOne - 1
};

struct FZenPackageVersioningInfo
{
	EZenPackageVersion ZenVersion;
	FPackageFileVersion PackageVersion;
	int32_t LicenseeVersion;
	FCustomVersionContainer CustomVersions;

	friend FArchive& operator<<(FArchive& Ar, FZenPackageVersioningInfo& ExportBundleEntry);
};

struct FZenPackageSummary
{
	uint32_t bHasVersioningInfo;
	uint32_t HeaderSize;
	FMappedName Name;
	uint32_t PackageFlags;
	uint32_t CookedHeaderSize;
	int32_t ImportedPublicExportHashesOffset;
	int32_t ImportMapOffset;
	int32_t ExportMapOffset;
	int32_t ExportBundleEntriesOffset;
	int32_t GraphDataOffset;
};

struct FZenPackageHeaderData
{
	uint32_t ExportCount = 0; 
	FZenPackageVersioningInfo VersioningInfo;
	FNameMap NameMap;
	std::string PackageName;
	FZenPackageSummary PackageSummary;
	std::vector<uint64_t> ImportedPublicExportHashes;
	std::vector<FPackageObjectIndex> ImportMap;
	std::vector<FExportMapEntry> ExportMap;
	std::vector<FPackageId> ImportedPackageIds;
	std::vector<FName> ImportedPackageNames;
	std::vector<FExportBundleHeader> ExportBundleHeaders;
	std::vector<FExportBundleEntry> ExportBundleEntries;
	uint8_t* AllExportDataPtr;

	__forceinline bool HasFlags(uint32_t Flags)
	{
		return PackageSummary.PackageFlags & Flags;
	}
};