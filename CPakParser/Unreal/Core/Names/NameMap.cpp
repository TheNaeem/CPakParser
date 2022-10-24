#include "NameMap.h"

const std::string FNameMap::GetName(FMappedName& MappedName) const
{
	auto Idx = MappedName.GetIndex();

	if (Idx >= NameEntries.size())
		return {};

	return NameEntries[Idx];
}

void FNameMap::Serialize(class FArchive& Ar, FMappedName::EType NameMapType)
{
	NameEntries = LoadNameBatch(Ar);
	this->NameMapType = NameMapType;
}