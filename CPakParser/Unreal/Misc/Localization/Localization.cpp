#include "Localization.h"

#include "../Guid.h"
#include "Serialization/Archives.h"

constexpr FGuid LOCRES_MAGIC = FGuid(0x7574140E, 0xFC034A67, 0x9D90154A, 0x1B7F37C3);
constexpr FGuid LOCMETA_MAGIC = FGuid(0xA14CEE4F, 0x83554868, 0xBD464C6C, 0x7C50DA70);

void FLocalization::Serialize(FSharedAr ArPtr)// TODO: optimize this instead of making it 1:1 with UE source
{
	auto& Ar = *ArPtr;

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
		return;
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

		Entries.reserve(Entries.size() + EntriesCount);
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

			Entries.insert_or_assign(FTextId(Namespace, Key), Val);
		}
	}

	return;
}