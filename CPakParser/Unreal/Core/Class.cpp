#include "Class.h"

#include "Serialization/Impl/ExportReader.h"
#include "Serialization/Properties/Unversioned.h"

UStruct::~UStruct()
{
	for (size_t i = 0; i < Properties.size(); i++)
	{
		auto Prop = Properties[i];
		auto ArraySize = Prop->GetArrayDim();

		delete Prop;

		i += ArraySize - 1;
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

std::vector<FProperty*> UStruct::GetProperties()
{
	return Properties;
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