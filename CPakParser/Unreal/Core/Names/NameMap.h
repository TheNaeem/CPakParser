#pragma once

#include "MappedName.h"
#include <vector>

class FNameMap
{
public:

	__forceinline int32_t Num() const
	{
		return NameEntries.size();
	}

	const std::string GetName(FMappedName& MappedName) const;
	void Serialize(class FArchive& Ar, FMappedName::EType NameMapType);

private:

	std::vector<std::string> NameEntries;
	FMappedName::EType NameMapType = FMappedName::EType::Global;
};