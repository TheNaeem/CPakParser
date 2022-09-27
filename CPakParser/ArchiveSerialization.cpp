#include "GameFileManager.h"
#include "Archives.h"
#include "PakFiles.h"
#include "IoContainer.h"
#include "Hashing.h"
#include "PackageSummary.h"

FArchive& operator<<(FArchive& Ar, std::string& InString)
{
	int32_t SaveNum = 0;
	Ar << SaveNum;

	bool bLoadUnicodeChar = SaveNum < 0;
	if (bLoadUnicodeChar) SaveNum = -SaveNum;

	if (!SaveNum) return Ar;

	if (bLoadUnicodeChar)
	{
		auto WStringData = std::make_unique<wchar_t[]>(SaveNum);
		Ar.Serialize(WStringData.get(), SaveNum * sizeof(wchar_t));

		auto Temp = std::wstring(WStringData.get());
		InString.assign(Temp.begin(), Temp.end());
	}
	else
	{
		InString.resize(SaveNum);
		Ar.Serialize(&InString[0], SaveNum);
	}

	return Ar;
}

FArchive& operator<<(FArchive& Ar, int32_t& InNum)
{
	Ar.Serialize(&InNum, sizeof(InNum));

	return Ar;
}

FArchive& operator<<(FArchive& Ar, uint32_t& InNum)
{
	Ar.Serialize(&InNum, sizeof(InNum));

	return Ar;
}

FArchive& operator<<(FArchive& Ar, uint64_t& InNum)
{
	Ar.Serialize(&InNum, sizeof(InNum));

	return Ar;
}

FArchive& operator<<(FArchive& Ar, int64_t& InNum)
{
	Ar.Serialize(&InNum, sizeof(InNum));

	return Ar;
}

FArchive& operator<<(FArchive& Ar, uint8_t& InByte)
{
	Ar.Serialize(&InByte, sizeof(InByte));

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FIoContainerId& ContainerId)
{
	Ar << ContainerId.Id;

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FPackageId& Value)
{
	Ar << Value.Id;

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FMappedName& MappedName)
{
	Ar << MappedName.Index << MappedName.Number;

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FIoContainerHeaderLocalizedPackage& LocalizedPackage)
{
	Ar << LocalizedPackage.SourcePackageId;
	Ar << LocalizedPackage.SourcePackageName;

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FIoContainerHeaderPackageRedirect& Redirect)
{
	Ar << Redirect.SourcePackageId;
	Ar << Redirect.TargetPackageId;
	Ar << Redirect.SourcePackageName;

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FIoContainerHeader& ContainerHeader)
{
	uint32_t Signature = FIoContainerHeader::Signature;
	Ar << Signature;

	if (Signature != FIoContainerHeader::Signature)
	{
		ReadStatus(ReadErrorCode::CorruptFile, "FIoContainerHeader signature read does not match the correct one.");
		Ar.SetError(true);
		return Ar;
	}

	EIoContainerHeaderVersion Version = EIoContainerHeaderVersion::Latest;
	Ar.Serialize(&Version, sizeof(Version));

	Ar << ContainerHeader.ContainerId;
	Ar.BulkSerializeArray(ContainerHeader.PackageIds);
	Ar << ContainerHeader.StoreEntries;
	Ar.BulkSerializeArray(ContainerHeader.OptionalSegmentPackageIds);
	Ar << ContainerHeader.OptionalSegmentStoreEntries;

	ContainerHeader.RedirectsNameMap = LoadNameBatch(Ar);

	Ar << ContainerHeader.LocalizedPackages;
	Ar << ContainerHeader.PackageRedirects;

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FGuid& Value)
{
	return Ar << Value.A << Value.B << Value.C << Value.D;
}

FArchive& operator<<(FArchive& Ar, bool& InBool)
{
	uint32_t UBool;
	Ar.Serialize(&UBool, sizeof(UBool));

	InBool = UBool;

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FSHAHash& G)
{
	Ar.Serialize(&G.Hash, sizeof(G.Hash));
	return Ar;
}

FArchive& operator<<(FArchive& Ar, FIoChunkHash& ChunkHash)
{
	Ar.Serialize(&ChunkHash.Hash, sizeof(ChunkHash.Hash));
	return Ar;
}

FArchive& operator<<(FArchive& Ar, FFileEntryInfo& Info)
{
	return Ar << Info.Entry.PakIndex;
}

void FGameFileManager::SerializePakIndexes(FArchive& Ar, std::string& MountPoint, std::shared_ptr<FPakFile> AssociatedPak)
{
	auto& DirectoryIndex = Get().FileLibrary;

	int32_t NewNumElements = 0;
	Ar << NewNumElements;

	if (!NewNumElements) return;

	DirectoryIndex.reserve(NewNumElements);

	for (size_t i = 0; i < NewNumElements; i++)
	{
		/*
		* FYI: I don't like this either.
		* But it's important for assigning the FFileEntryInfo as an FPakEntryLocation,
		* as well as for assigning the shared pak pointer.
		* I would of course prefer reducing this to much less lines and making it cleaner by just directly serializing the FPakDirectory.
		* But it is what it is.
		* Will revisit this another time, so it is a TODO, but for now this will do. At least it doesn't affect speed.
		*/

		std::string DirectoryName;
		FPakDirectory DirIdx;

		Ar << DirectoryName;

		int32_t DirIdxNum = 0;
		Ar << DirIdxNum;

		if (!DirIdxNum) continue;

		DirectoryName = MountPoint + DirectoryName;
		DirIdx.reserve(DirIdxNum);

		for (size_t i = 0; i < DirIdxNum; i++)
		{
			auto File = std::pair<std::string, FPakEntryLocation>();
			Ar << File;

			File.second.SetOwningFile(AssociatedPak);

			DirIdx.insert_or_assign(File.first, File.second);
		}

		if (!DirectoryIndex.contains(DirectoryName))
		{
			DirectoryIndex.insert_or_assign(DirectoryName, DirIdx);
			continue;
		}

		DirectoryIndex[DirectoryName].merge(DirIdx);
	}
}

FArchive& operator<<(FArchive& Ar, FEngineVersion& Version)
{
	Ar << Version.Branch;

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FGenerationInfo& Info)
{
	Ar << Info.ExportCount;
	Ar << Info.NameCount;

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FPackageFileSummary& Summary)
{
	Ar << Summary.Tag;

	if (Summary.Tag != PACKAGE_FILE_TAG && Summary.Tag != PACKAGE_FILE_TAG_SWAPPED)
		return Ar;

	if (Summary.Tag == PACKAGE_FILE_TAG_SWAPPED)
	{
		Summary.Tag = PACKAGE_FILE_TAG;

		ReadStatus(ReadErrorCode::Unknown, "Package summary must be endian swapped. TODO");
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

	if (Summary.PackageFlags & PKG_FilterEditorOnly)
	{
		Ar.ArIsFilterEditorOnly = true;
	}

	Ar << Summary.NameCount;
	Ar << Summary.NameOffset;

	if (!Ar.ArIsFilterEditorOnly &&
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