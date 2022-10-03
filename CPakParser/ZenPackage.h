#pragma once

#include "CoreTypes.h"
#include "Package.h"

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

class FPackageObjectIndex
{
	static constexpr uint64_t IndexBits = 62ull;
	static constexpr uint64_t IndexMask = (1ull << IndexBits) - 1ull;
	static constexpr uint64_t TypeMask = ~IndexMask;
	static constexpr uint64_t TypeShift = IndexBits;
	static constexpr uint64_t Invalid = ~0ull;

	uint64_t TypeAndId = Invalid;

	enum EType
	{
		Export,
		ScriptImport,
		PackageImport,
		Null,
		TypeCount = Null,
	};

	__forceinline explicit FPackageObjectIndex(EType InType, uint64_t InId) : TypeAndId((uint64_t(InType) << TypeShift) | InId) {}

public:
	FPackageObjectIndex() = default;

	__forceinline static FPackageObjectIndex FromExportIndex(const int32_t Index)
	{
		return FPackageObjectIndex(Export, Index);
	}

	__forceinline bool IsNull() const
	{
		return TypeAndId == Invalid;
	}

	__forceinline bool IsExport() const
	{
		return (TypeAndId >> TypeShift) == Export;
	}

	__forceinline bool IsImport() const
	{
		return IsScriptImport() || IsPackageImport();
	}

	__forceinline bool IsScriptImport() const
	{
		return (TypeAndId >> TypeShift) == ScriptImport;
	}

	__forceinline bool IsPackageImport() const
	{
		return (TypeAndId >> TypeShift) == PackageImport;
	}

	__forceinline uint32_t ToExport() const
	{
		return uint32_t(TypeAndId);
	}

	__forceinline uint64_t Value() const
	{
		return TypeAndId & IndexMask;
	}

	__forceinline bool operator==(FPackageObjectIndex Other) const
	{
		return TypeAndId == Other.TypeAndId;
	}

	__forceinline bool operator!=(FPackageObjectIndex Other) const
	{
		return TypeAndId != Other.TypeAndId;
	}

	__forceinline friend FArchive& operator<<(FArchive& Ar, FPackageObjectIndex& Value)
	{
		return Ar << Value.TypeAndId;
	}

	__forceinline friend uint32_t hash_value(const FPackageObjectIndex& Value)
	{
		return uint32_t(Value.TypeAndId);
	}
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
	EObjectFlags ObjectFlags = EObjectFlags::RF_NoFlags;
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
};