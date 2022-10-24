#include "PackageSummary.h"

#include "Serialization/Archives.h"
#include "Logger.h"
#include "PackageFlags.h"
#include "Misc/Compression/GlobalCompression.h"

FArchive& operator<<(FArchive& Ar, FPackageFileSummary& Summary)
{
	Ar << Summary.Tag;

	if (Summary.Tag != PACKAGE_FILE_TAG && Summary.Tag != PACKAGE_FILE_TAG_SWAPPED)
		return Ar;

	if (Summary.Tag == PACKAGE_FILE_TAG_SWAPPED)
	{
		Summary.Tag = PACKAGE_FILE_TAG;

		Log<Warning>("Package summary must be endian swapped. TODO");
	}

	constexpr int32_t CurrentLegacyFileVersion = -8;
	int32_t LegacyFileVersion = CurrentLegacyFileVersion;
	Ar << LegacyFileVersion;

	if (LegacyFileVersion >= 0 || LegacyFileVersion < CurrentLegacyFileVersion)
		return Ar;

	if (LegacyFileVersion != -4)
		Ar.SeekCur<int32_t>();

	Ar << Summary.FileVersionUE.FileVersionUE4;

	if (LegacyFileVersion <= -8)
		Ar << Summary.FileVersionUE.FileVersionUE5;

	Ar << Summary.FileVersionLicenseeUE;

	if (!Summary.FileVersionUE.FileVersionUE4 &&
		!Summary.FileVersionUE.FileVersionUE5 &&
		!Summary.FileVersionLicenseeUE)
	{
		Summary.bUnversioned = true;

		// TODO
		//Sum.FileVersionUE = GPackageFileUEVersion;
		//Sum.FileVersionLicenseeUE = GPackageFileLicenseeUEVersion;
		//Sum.CustomVersionContainer = FCurrentCustomVersions::GetAll();
	}

	Ar << Summary.TotalHeaderSize;
	Ar << Summary.FolderName;
	Ar << Summary.PackageFlags;

	Ar << Summary.NameCount;
	Ar << Summary.NameOffset;

	if (!(Summary.PackageFlags & PKG_FilterEditorOnly) &&
		Summary.FileVersionUE >= VER_UE4_ADDED_PACKAGE_SUMMARY_LOCALIZATION_ID)
	{
		Ar << Summary.LocalizationId;
	}

	if (Summary.FileVersionUE >= VER_UE4_SERIALIZE_TEXT_IN_PACKAGES)
	{
		Ar << Summary.GatherableTextDataCount;
		Ar << Summary.GatherableTextDataOffset;
	}

	Ar << Summary.ExportCount << Summary.ExportOffset;
	Ar << Summary.ImportCount << Summary.ImportOffset;
	Ar << Summary.DependsOffset;

	// TODO: IsFileVersionTooNew
	if (Summary.FileVersionUE < VER_UE4_OLDEST_LOADABLE_PACKAGE)
		return Ar;

	if (Summary.FileVersionUE >= VER_UE4_ADD_STRING_ASSET_REFERENCES_MAP)
	{
		Ar << Summary.SoftPackageReferencesCount;
		Ar << Summary.SoftPackageReferencesOffset;
	}

	if (Summary.FileVersionUE >= VER_UE4_ADDED_SEARCHABLE_NAMES)
	{
		Ar << Summary.SearchableNamesOffset;
	}

	Ar << Summary.ThumbnailTableOffset;
	Ar << Summary.Guid;

	Ar.BulkSerializeArray(Summary.Generations);

	if (Summary.FileVersionUE >= VER_UE4_ENGINE_VERSION_OBJECT)
	{
		Ar << Summary.SavedByEngineVersion;
		FixCorruptEngineVersion(Summary.FileVersionUE, Summary.SavedByEngineVersion);
	}
	else
	{
		int32_t EngineChangelist = 0;
		Ar << EngineChangelist;

		if (EngineChangelist)
			Summary.SavedByEngineVersion.Set(4, 0, 0, EngineChangelist, {});
	}

	if (Summary.FileVersionUE >= VER_UE4_PACKAGE_SUMMARY_HAS_COMPATIBLE_ENGINE_VERSION)
	{
		Ar << Summary.CompatibleWithEngineVersion;
		FixCorruptEngineVersion(Summary.FileVersionUE, Summary.CompatibleWithEngineVersion);
	}
	else Summary.CompatibleWithEngineVersion = Summary.SavedByEngineVersion;

	Ar << Summary.CompressionFlags;

	constexpr int32_t CompressionFlagsMask = COMPRESS_DeprecatedFormatFlagsMask | COMPRESS_OptionsFlagsMask | COMPRESS_ForPurposeMask;

	if (Summary.CompressionFlags & (~CompressionFlagsMask))
		return Ar;

	int32_t ChunksCount;
	Ar << ChunksCount;

	if (ChunksCount)
		return Ar;

	Ar << Summary.PackageSource;

	std::vector<std::string> AdditionalPackagesToCook;
	Ar << AdditionalPackagesToCook;

	if (LegacyFileVersion > -7)
	{
		Ar.SeekCur<int32_t>();
	}

	Ar << Summary.AssetRegistryDataOffset;
	Ar << Summary.BulkDataStartOffset;

	if (Summary.FileVersionUE >= VER_UE4_WORLD_LEVEL_INFO)
	{
		Ar << Summary.WorldTileInfoDataOffset;
	}

	if (Summary.FileVersionUE >= VER_UE4_CHANGED_CHUNKID_TO_BE_AN_ARRAY_OF_CHUNKIDS)
	{
		Ar << Summary.ChunkIDs;
	}
	else if (Summary.FileVersionUE >= VER_UE4_ADDED_CHUNKID_TO_ASSETDATA_AND_UPACKAGE)
	{
		int ChunkID = -1;
		Ar << ChunkID;

		if (ChunkID >= 0)
		{
			Summary.ChunkIDs.push_back(ChunkID);
		}
	}

	if (Summary.FileVersionUE >= VER_UE4_PRELOAD_DEPENDENCIES_IN_COOKED_EXPORTS)
	{
		Ar << Summary.PreloadDependencyCount;
		Ar << Summary.PreloadDependencyOffset;
	}
	else
	{
		Summary.PreloadDependencyCount = -1;
		Summary.PreloadDependencyOffset = 0;
	}

	if (Summary.FileVersionUE >= EUnrealEngineObjectUE5Version::NAMES_REFERENCED_FROM_EXPORT_DATA)
	{
		Ar << Summary.NamesReferencedFromExportDataCount;
	}
	else
	{
		Summary.NamesReferencedFromExportDataCount = Summary.NameCount;
	}

	if (Summary.FileVersionUE >= EUnrealEngineObjectUE5Version::PAYLOAD_TOC)
	{
		Ar << Summary.PayloadTocOffset;
	}
	else Summary.PayloadTocOffset = INDEX_NONE;

	return Ar;
}