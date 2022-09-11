#include "GameFileManager.h"
#include "Archives.h"
#include "PakFiles.h"
#include "IoContainer.h"
#include "Hashing.h"

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