#include "PackageFileVersion.h"

#include "Serialization/Archives.h"
#include "EngineVersion.h"

FPackageFileVersion FPackageFileVersion::CreateUE4Version(__int32 Version)
{
	return FPackageFileVersion(Version, (EUnrealEngineObjectUE5Version)0);
}

FPackageFileVersion FPackageFileVersion::CreateUE4Version(EUnrealEngineObjectUE4Version Version)
{
	return FPackageFileVersion(Version, (EUnrealEngineObjectUE5Version)0);
}

bool FPackageFileVersion::operator !=(EUnrealEngineObjectUE4Version Version) const
{
	return FileVersionUE4 != Version;
}

bool FPackageFileVersion::operator <(EUnrealEngineObjectUE4Version Version) const
{
	return FileVersionUE4 < Version;
}

bool FPackageFileVersion::operator >=(EUnrealEngineObjectUE4Version Version) const
{
	return FileVersionUE4 >= Version;
}

bool FPackageFileVersion::operator !=(EUnrealEngineObjectUE5Version Version) const
{
	return FileVersionUE5 != (int32_t)Version;
}

bool FPackageFileVersion::operator <(EUnrealEngineObjectUE5Version Version) const
{
	return FileVersionUE5 < (int32_t)Version;
}

bool FPackageFileVersion::operator >=(EUnrealEngineObjectUE5Version Version) const
{
	return FileVersionUE5 >= (int32_t)Version;
}

bool FPackageFileVersion::operator==(const FPackageFileVersion& Other) const
{
	return FileVersionUE4 == Other.FileVersionUE4 && FileVersionUE5 == Other.FileVersionUE5;
}

bool FPackageFileVersion::operator!=(const FPackageFileVersion& Other) const
{
	return !(*this == Other);
}

void FPackageFileVersion::Reset()
{
	FileVersionUE4 = 0;
	FileVersionUE5 = 0;
}

int32_t FPackageFileVersion::ToValue() const
{
	if (FileVersionUE5 >= (int32_t)EUnrealEngineObjectUE5Version::INITIAL_VERSION)
	{
		return FileVersionUE5;
	}
	else
	{
		return FileVersionUE4;
	}
}

bool FPackageFileVersion::IsCompatible(const FPackageFileVersion& Other) const
{
	return FileVersionUE4 >= Other.FileVersionUE4 && FileVersionUE5 >= Other.FileVersionUE5;
}

FArchive& operator<<(FArchive& Ar, FPackageFileVersion& Version)
{
	Ar.Serialize(&Version.FileVersionUE4, sizeof(__int32) * 2);

	return Ar;
}

void FixCorruptEngineVersion(const FPackageFileVersion& ObjectVersion, FEngineVersion& Version)
{
	if (ObjectVersion < VER_UE4_CORRECT_LICENSEE_FLAG
		&& Version.GetMajor() == 4
		&& Version.GetMinor() == 26
		&& Version.GetPatch() == 0
		&& Version.GetChangelist() >= 12740027
		&& Version.IsLicenseeVersion())
	{
		Version.Set(4, 26, 0, Version.GetChangelist(), Version.GetBranch());
	}
}