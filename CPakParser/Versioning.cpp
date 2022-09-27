#include "Versioning.h"

FCustomVersion::FCustomVersion(FGuid InKey, int32_t InVersion, FName InFriendlyName)
	: Key(InKey)
	, Version(InVersion)
	, ReferenceCount(1)
	, FriendlyName(InFriendlyName)
{
}

FEngineVersionBase::FEngineVersionBase(uint16_t InMajor, uint16_t InMinor, uint16_t InPatch , uint32_t InChangelist )
	: Major(InMajor), Minor(InMinor), Patch(InPatch), Changelist(InChangelist)
{
}

uint16_t FEngineVersionBase::GetMajor() const
{
	return Major;
}

uint16_t FEngineVersionBase::GetMinor() const
{
	return Minor;
}

uint16_t FEngineVersionBase::GetPatch() const
{
	return Patch;
}

uint16_t FEngineVersionBase::GetChangelist() const
{
	return Changelist & (uint32_t)0x7fffffffU;
}

bool FEngineVersionBase::IsLicenseeVersion() const
{
	return (Changelist & (uint32_t)0x80000000U) != 0;
}

const std::string FEngineVersion::GetBranch() const
{
	return Branch;
}

void FEngineVersion::Set(uint16_t InMajor, uint16_t InMinor, uint16_t InPatch, uint32_t InChangelist, const std::string& InBranch)
{
	Major = InMajor;
	Minor = InMinor;
	Patch = InPatch;
	Changelist = InChangelist;
	Branch = InBranch;
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

bool FCustomVersion::operator==(FGuid InKey) const
{
	return Key == InKey;
}

bool FCustomVersion::operator!=(FGuid InKey) const
{
	return Key != InKey;
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