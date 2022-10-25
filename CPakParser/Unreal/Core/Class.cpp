#include "Class.h"

#include "Serialization/Impl/ExportReader.h"
#include "Serialization/Properties/Unversioned.h"

UStructPtr UStruct::GetSuper()
{
	return Super;
}

void UStruct::SetSuper(UStructPtr Val)
{
	Super = Val;
}

void UStruct::SerializeScriptProperties(TSharedPtr<FExportReader> Ar, uint8_t* Data, UStructPtr DefaultsStruct, uint8_t* Defaults)
{
	if (Ar->UseUnversionedPropertySerialization())
	{
		UnversionedSerializer::SerializeUnversionedProperties(This<UStruct>(), Ar, Data, DefaultsStruct, Defaults);
		return;
	}

	// TODO: SerializeVersionedTaggedProperties
}