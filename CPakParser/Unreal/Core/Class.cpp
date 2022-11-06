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

void UStruct::SerializeScriptProperties(FArchive& Ar, UObjectPtr Object)
{
	if (Ar.UseUnversionedPropertySerialization())
	{
		FUnversionedSerializer::SerializeUnversionedProperties(This<UStruct>(), Ar, Object);
		return;
	}

	// TODO: SerializeVersionedTaggedProperties
}