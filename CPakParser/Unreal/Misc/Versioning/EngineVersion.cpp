#include "EngineVersion.h"

#include "Serialization/Archives.h"

FEngineVersionBase::FEngineVersionBase(uint16_t InMajor, uint16_t InMinor, uint16_t InPatch, uint32_t InChangelist)
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

FArchive& operator<<(FArchive& Ar, FEngineVersion& Version)
{
	Ar << Version.Branch;

	return Ar;
}