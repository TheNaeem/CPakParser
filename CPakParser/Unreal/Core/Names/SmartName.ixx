module;

#include "Core/Defines.h"

export module CPakParser.Names.SmartName;

export import CPakParser.Core.FName;
import CPakParser.Serialization.FArchive;
import CPakParser.Versioning.Custom.AnimPhysObjectVersion;

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

	friend FArchive& operator<<(FArchive& Ar, FSmartName& SmartName)
	{
		Ar << SmartName.DisplayName;

		if (Ar.CustomVer(FAnimPhysObjectVersion::GUID) < FAnimPhysObjectVersion::RemoveUIDFromSmartNameSerialize)
		{
			Ar.SeekCur<uint16_t>();
		}

		if (Ar.CustomVer(FAnimPhysObjectVersion::GUID) < FAnimPhysObjectVersion::SmartNameRefactorForDeterministicCooking)
		{
			Ar.SeekCur<FGuid>();
		}

		return Ar;
	}
};