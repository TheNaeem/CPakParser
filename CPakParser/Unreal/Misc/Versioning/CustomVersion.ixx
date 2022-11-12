export module CustomVersion;

import <vector>;
import FArchiveBase;
import Guid;

export typedef std::vector<struct FCustomVersion> FCustomVersionArray;

enum class ECustomVersionSerializationFormat
{
	Unknown,
	Guids,
	Enums,
	Optimized,

	CustomVersion_Automatic_Plus_One,
	Latest = CustomVersion_Automatic_Plus_One - 1
};

struct FCustomVersion
{
	FCustomVersion() = default;

	friend class FCustomVersionContainer;

	FGuid Key;
	int32_t Version;
	int32_t ReferenceCount;

	bool operator==(FGuid InKey) const
	{
		return Key == InKey;
	}

	bool operator!=(FGuid InKey) const
	{
		return Key != InKey;
	}

	friend FArchive& operator<<(FArchive& Ar, FCustomVersion& Version)
	{
		/*
		Ar << Version.Key;
		Ar << Version.Version;
		*/

		Ar.Serialize(&Version.Key, sizeof(Version.Key) + sizeof(Version.Version));

		return Ar;
	}
};

class FCustomVersionContainer
{
public:

	void Serialize(FArchive& Ar, ECustomVersionSerializationFormat Format = ECustomVersionSerializationFormat::Latest)
	{
		switch (Format) // implement the deprecated formats if needed but i doubt it ever will
		{
		case ECustomVersionSerializationFormat::Optimized:
		{
			Ar << Versions;
		}
		break;
		}
	}

private:

	FCustomVersionArray Versions;
};