#include "IoContainerHeader.h"

#include "Serialization/Archives.h"
#include "Logger.h"

enum class EIoContainerHeaderVersion : uint32_t
{
	Initial = 0,
	LocalizedPackages = 1,
	OptionalSegmentPackages = 2,

	LatestPlusOne,
	Latest = LatestPlusOne - 1
};

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

	ContainerHeader.RedirectsNameMap = LoadNameBatch(Ar); // TODO: what is this for?

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