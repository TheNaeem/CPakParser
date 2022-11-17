export module CPakParser.Versions.EngineVersion;

import <string>;
import CPakParser.Serialization.FArchive;

export class FEngineVersionBase
{
public:

	FEngineVersionBase() = default;

	FEngineVersionBase(uint16_t InMajor, uint16_t InMinor, uint16_t InPatch, uint32_t InChangelist)
		: Major(InMajor), Minor(InMinor), Patch(InPatch), Changelist(InChangelist)
	{
	}

	uint16_t GetMajor() const
	{
		return Major;
	}

	uint16_t GetMinor() const
	{
		return Minor;
	}

	uint16_t GetPatch() const
	{
		return Patch;
	}

	uint16_t GetChangelist() const
	{
		return Changelist & (uint32_t)0x7fffffffU;
	}

	bool IsLicenseeVersion() const
	{
		return (Changelist & (uint32_t)0x80000000U) != 0;
	}

protected:

	uint16_t Major = 0;
	uint16_t Minor = 0;
	uint16_t Patch = 0;
	uint32_t Changelist = 0;
};

export class FEngineVersion : public FEngineVersionBase
{
public:

	friend FArchive& operator<<(FArchive& Ar, FEngineVersion& Version)
	{
		return Ar << Version.Branch;
	}

	const std::string GetBranch() const
	{
		return Branch;
	}

	void Set(uint16_t InMajor, uint16_t InMinor, uint16_t InPatch, uint32_t InChangelist, const std::string& InBranch)
	{
		Major = InMajor;
		Minor = InMinor;
		Patch = InPatch;
		Changelist = InChangelist;
		Branch = InBranch;
	}

private:

	std::string Branch;
};