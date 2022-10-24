#pragma once

#include <string>

class FEngineVersionBase
{
public:

	FEngineVersionBase() = default;
	FEngineVersionBase(uint16_t InMajor, uint16_t InMinor, uint16_t InPatch = 0, uint32_t InChangelist = 0);

	uint16_t GetMajor() const;
	uint16_t GetMinor() const;
	uint16_t GetPatch() const;
	uint16_t GetChangelist() const;
	bool IsLicenseeVersion() const;

protected:

	uint16_t Major = 0;
	uint16_t Minor = 0;
	uint16_t Patch = 0;
	uint32_t Changelist = 0;
};

class FEngineVersion : public FEngineVersionBase
{
public:

	friend class FArchive& operator<<(class FArchive& Ar, FEngineVersion& Version);

	const std::string GetBranch() const;
	void Set(uint16_t InMajor, uint16_t InMinor, uint16_t InPatch, uint32_t InChangelist, const std::string& InBranch);

private:

	std::string Branch;
};