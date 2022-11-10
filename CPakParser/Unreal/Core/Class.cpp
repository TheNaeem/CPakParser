#include "Class.h"

#include "Serialization/Impl/ExportReader.h"
#include "Serialization/Properties/Unversioned.h"
#include "Logging.h"

UStruct::~UStruct()
{
	while (PropertyLink)
	{
		auto LinkCopy = PropertyLink;
		PropertyLink = PropertyLink->GetNext();
		delete LinkCopy;
	}
}

UStructPtr UStruct::GetSuper()
{
	return Super;
}

void UStruct::SetSuper(UStructPtr Val)
{
	Super = Val;
}

void UStruct::SerializeScriptProperties(FArchive& Ar, UObjectPtr Object)
{
	if (Ar.UseUnversionedPropertySerialization())
	{
		FUnversionedSerializer::SerializeUnversionedProperties(This<UStruct>(), Ar, Object);
		return;
	}
	else
	{
		LogError("SerializeVersionedTaggedProperties not yet implemented.");
	}
	// TODO: SerializeVersionedTaggedProperties
}