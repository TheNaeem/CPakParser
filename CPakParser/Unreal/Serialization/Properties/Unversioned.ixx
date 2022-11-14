export module Unversioned;

import FArchiveBase;
import UObjectCore;

export struct FUnversionedSerializer
{
	static void SerializeUnversionedProperties(UStructPtr Struct, FArchive& Ar, UObjectPtr Object);
};