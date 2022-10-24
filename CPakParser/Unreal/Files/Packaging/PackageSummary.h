#pragma once

#include "Misc/Guid.h"
#include "Misc/Versioning/EngineVersion.h"
#include "Misc/Versioning/PackageFileVersion.h"
#include "Misc/Versioning/CustomVersion.h"
#include <string>
#include <vector>

#define PACKAGE_FILE_TAG 0x9E2A83C1
#define PACKAGE_FILE_TAG_SWAPPED 0xC1832A9E

struct FGenerationInfo
{
	int32_t ExportCount;
	int32_t NameCount;

	friend class FArchive& operator<<(FArchive& Ar, FGenerationInfo& Info);
};

struct FPackageFileSummary
{
	int32_t	Tag;
	FPackageFileVersion FileVersionUE;
	int32_t	FileVersionLicenseeUE;
	FCustomVersionContainer CustomVersionContainer;
	uint32_t PackageFlags;
	int32_t	TotalHeaderSize;
	std::string	FolderName;
	int32_t	NameCount;
	int32_t NameOffset;
	std::string LocalizationId;
	int32_t	GatherableTextDataCount;
	int32_t GatherableTextDataOffset;
	int32_t ExportCount;
	int32_t ExportOffset;
	int32_t	ImportCount;
	int32_t	ImportOffset;
	int32_t DependsOffset;
	int32_t SoftPackageReferencesCount;
	int32_t SoftPackageReferencesOffset;
	int32_t SearchableNamesOffset;
	int32_t ThumbnailTableOffset;
	FGuid Guid;
	std::vector<FGenerationInfo> Generations;
	FEngineVersion SavedByEngineVersion;
	FEngineVersion CompatibleWithEngineVersion;
	uint32_t CompressionFlags;
	uint32_t PackageSource;
	bool bUnversioned;
	int32_t AssetRegistryDataOffset;
	int64_t	BulkDataStartOffset;
	int32_t WorldTileInfoDataOffset;
	std::vector<int32_t> ChunkIDs;
	int32_t	PreloadDependencyCount;
	int32_t PreloadDependencyOffset;
	int32_t	NamesReferencedFromExportDataCount;
	int64_t PayloadTocOffset;

	friend FArchive& operator<<(FArchive& Ar, FPackageFileSummary& Summary);
};