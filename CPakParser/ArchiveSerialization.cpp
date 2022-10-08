#include "GameFileManager.h"
#include "Localization.h"
#include "PakFiles.h"
#include "IoContainer.h"
#include "Hashing.h"
#include "PackageSummary.h"
#include "ZenPackage.h"

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
		Log<Warning>("FIoContainerHeader signature read does not match the correct one.");
		return Ar;
	}

	EIoContainerHeaderVersion Version = EIoContainerHeaderVersion::Latest;
	Ar.Serialize(&Version, sizeof(Version));

	Ar << ContainerHeader.ContainerId;
	Ar.BulkSerializeArray(ContainerHeader.PackageIds);

	auto PackageCount = ContainerHeader.PackageIds.size();

	Ar << ContainerHeader.StoreEntriesData;

	Ar.BulkSerializeArray(ContainerHeader.OptionalSegmentPackageIds);
	Ar << ContainerHeader.OptionalSegmentStoreEntries;

	ContainerHeader.RedirectsNameMap = LoadNameBatch(Ar);

	Ar.BulkSerializeArray(ContainerHeader.LocalizedPackages);
	Ar.BulkSerializeArray(ContainerHeader.PackageRedirects);

	auto StoreEntries = (FFilePackageStoreEntry*)ContainerHeader.StoreEntriesData.data();
	ContainerHeader.PackageStore.reserve(PackageCount);

	for (size_t i = 0; i < PackageCount; i++)
	{
		ContainerHeader.PackageStore.insert_or_assign(ContainerHeader.PackageIds[i], StoreEntries[i]);
	}

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FGuid& Value)
{
	Ar.Serialize(&Value.A, sizeof(FGuid));
	return Ar;
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

void FGameFileManager::SerializePakIndexes(FArchive& Ar, std::string& MountPoint, TSharedPtr<FPakFile> AssociatedPak)
{
	int32_t NewNumElements = 0;
	Ar << NewNumElements;

	if (!NewNumElements) return;

	FileLibrary.reserve(NewNumElements);

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

		DirectoryName.pop_back(); // remove the null terminator

		for (size_t i = 0; i < DirIdxNum; i++)
		{
			auto File = std::pair<std::string, FPakEntryLocation>();
			Ar << File;

			File.first.pop_back(); // remove the null terminator

			File.second.SetOwningFile(AssociatedPak);

			DirIdx.insert_or_assign(File.first, File.second);
		}

		if (!FileLibrary.contains(DirectoryName))
		{
			FileLibrary.insert_or_assign(DirectoryName, DirIdx);
			continue;
		}

		FileLibrary[DirectoryName].merge(DirIdx);
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

FArchive& operator<<(FArchive& Ar, FTextLocalizationResourceString& A)
{
	Ar << A.String;
	Ar.SeekCur<uint32_t>();

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FLocalization& Loc) // TODO: optimize this instead of making it 1:1 with UE source
{
	FGuid Magic;
	Ar << Magic;

	auto VersionNumber = ELocResVersion::Legacy;

	if (Magic != LOCRES_MAGIC)
	{
		Ar.Seek(0);
	}
	else
	{
		Ar.Serialize(&VersionNumber, sizeof(VersionNumber));
	}

	if (VersionNumber > ELocResVersion::Latest)
	{
		return Ar;
	}

	std::vector<FTextLocalizationResourceString> LocalizedStringArray;

	if (VersionNumber >= ELocResVersion::Compact)
	{
		int64_t LocalizedStringArrayOffset = INDEX_NONE;
		Ar << LocalizedStringArrayOffset;

		if (LocalizedStringArrayOffset != INDEX_NONE)
		{
			const auto CurrentFileOffset = Ar.Tell();
			Ar.Seek(LocalizedStringArrayOffset);

			if (VersionNumber >= ELocResVersion::Optimized_CRC32)
			{
				Ar << LocalizedStringArray;
			}
			else
			{
				std::vector<std::string> TmpLocalizedStringArray;
				Ar << TmpLocalizedStringArray;

				LocalizedStringArray.reserve(TmpLocalizedStringArray.size());

				for (auto LocalizedString : TmpLocalizedStringArray)
				{
					LocalizedStringArray.push_back(FTextLocalizationResourceString(LocalizedString));
				}
			}

			Ar.Seek(CurrentFileOffset);
		}
	}

	if (VersionNumber >= ELocResVersion::Optimized_CRC32)
	{
		uint32_t EntriesCount;
		Ar << EntriesCount;

		Loc.Entries.reserve(Loc.Entries.size() + EntriesCount);
	}

	uint32_t NamespaceCount;
	Ar << NamespaceCount;

	for (uint32_t i = 0; i < NamespaceCount; ++i)
	{
		FTextKey Namespace;
		Namespace.Serialize(Ar, VersionNumber);

		uint32_t KeyCount;
		Ar << KeyCount;

		for (uint32_t j = 0; j < KeyCount; ++j)
		{
			FTextKey Key;
			Key.Serialize(Ar, VersionNumber);

			Ar.SeekCur<uint32_t>();

			std::string Val;

			if (VersionNumber >= ELocResVersion::Compact)
			{
				int32_t LocalizedStringIndex = INDEX_NONE;
				Ar << LocalizedStringIndex;

				if (LocalizedStringIndex < LocalizedStringArray.size())
				{
					Val = LocalizedStringArray[LocalizedStringIndex].String;
				}
			}
			else
			{
				Ar << Val;
			}

			Loc.Entries.insert_or_assign(FTextId(Namespace, Key), Val);
		}
	}

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FZenPackageVersioningInfo& VersioningInfo)
{
	Ar.Serialize(&VersioningInfo.ZenVersion, sizeof(EZenPackageVersion));
	Ar << VersioningInfo.PackageVersion;
	Ar << VersioningInfo.LicenseeVersion;

	VersioningInfo.CustomVersions.Serialize(Ar);

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FCustomVersion& Version)
{
	/*
	Ar << Version.Key;
	Ar << Version.Version;
	*/

	Ar.Serialize(&Version.Key, sizeof(Version.Key) + sizeof(Version.Version));

	return Ar;
}

FArchive& operator<<(FArchive& Ar, FPackageFileVersion& Version)
{
	Ar << Version.FileVersionUE4;
	Ar << Version.FileVersionUE5;

	return Ar;
}