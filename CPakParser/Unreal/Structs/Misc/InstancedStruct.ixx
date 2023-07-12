export module InstancedStruct;

import CPakParser.Serialization.FArchive;
import CPakParser.Core.UObject;

export struct FInstancedStruct
{
public:

	FInstancedStruct() = default;

    friend inline FArchive& operator<<(FArchive& Ar, FInstancedStruct& Struct)
    {
		enum class EVersion : uint8_t
		{
			InitialVersion = 0,
			VersionPlusOne,
			LatestVersion = VersionPlusOne - 1
		};

		EVersion Version = EVersion::LatestVersion;

		Ar.Serialize(&Version, sizeof(Version));

		if (Version > EVersion::LatestVersion)
		{
			return Ar;
		}

		Ar << Struct.ScriptStruct;

		int32_t SerialSize = 0;
		Ar << SerialSize;

		if (!Struct.ScriptStruct and SerialSize > 0)
		{
			Ar.SeekCur(SerialSize);
		}
		else if (Struct.ScriptStruct)
		{
			Struct.ScriptStruct->SerializeItem(Ar);
		}
		
        return Ar;
    }

	UStructPtr ScriptStruct = nullptr;
};