module;

#include "Core/Defines.h"

export module CPakParser.Names.SmartName;

export import CPakParser.Core.FName;
import CPakParser.Serialization.FArchive;

export struct FSmartName
{
	FName DisplayName;
	uint16_t UID;

	FSmartName()
		: DisplayName({})
		, UID(MAX_uint16)
	{}

	FSmartName(const FName& InDisplayName, uint16_t InUID)
		: DisplayName(InDisplayName)
		, UID(InUID)
	{}

	bool operator==(FSmartName const& Other) const
	{
		return (DisplayName == Other.DisplayName && UID == Other.UID);
	}

	bool operator!=(const FSmartName& Other) const
	{
		return !(*this == Other);
	}

	bool IsValid() const
	{
		return UID != MAX_uint16;
	}
};