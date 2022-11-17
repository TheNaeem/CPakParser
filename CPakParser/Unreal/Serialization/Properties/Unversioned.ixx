export module CPakParser.Serialization.Unversioned;

import CPakParser.Serialization.FArchive;
import CPakParser.Core.UObject;

export struct FUnversionedSerializer
{
	static void SerializeUnversionedProperties(UStructPtr Struct, FArchive& Ar, UObjectPtr Object);
};