#pragma once

#include "Core/Defines.h"
#include "../Guid.h"
#include <vector>

typedef std::vector<struct FCustomVersion> FCustomVersionArray;

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

	__forceinline bool operator==(FGuid InKey) const;
	__forceinline bool operator!=(FGuid InKey) const;

	friend FArchive& operator<<(FArchive& Ar, FCustomVersion& Version);
};

class FCustomVersionContainer
{
public:

	void Serialize(FArchive& Ar, ECustomVersionSerializationFormat Format = ECustomVersionSerializationFormat::Latest);

private:

	FCustomVersionArray Versions;
};