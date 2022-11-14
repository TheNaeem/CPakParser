export module NameMap;

export import MappedName;
import Name;

export import <string>;
import <vector>;

export class FNameMap
{
public:

	__forceinline int32_t Num() const
	{
		return NameEntries.size();
	}

	const std::string GetName(FMappedName& MappedName) const
	{
		auto Idx = MappedName.GetIndex();

		if (Idx >= NameEntries.size())
			return {};

		return NameEntries[Idx];
	}

	void Serialize(class FArchive& Ar, FMappedName::EType NameMapType)
	{
		NameEntries = LoadNameBatch(Ar);
		this->NameMapType = NameMapType;
	}

private:

	std::vector<std::string> NameEntries;
	FMappedName::EType NameMapType = FMappedName::EType::Global;
};