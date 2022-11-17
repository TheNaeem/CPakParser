import CPakParser.Core.UObject;
import CPakParser.Serialization.FArchive;
import CPakParser.Logging;
import CPakParser.Serialization.Unversioned;

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