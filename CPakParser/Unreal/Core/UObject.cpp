import CPakParser.Core.UObject;
import CPakParser.Serialization.FArchive;

void UObject::Serialize(FArchive& Ar)
{
	/*if (Class->HasAnyFlags(RF_NeedLoad)) // TODO:
	{
		UnderlyingArchive.Preload(ObjClass);

		if (!HasAnyFlags(RF_ClassDefaultObject) && ObjClass->GetDefaultsCount() > 0)
		{
			UnderlyingArchive.Preload(ObjClass->GetDefaultObject());
		}
	}*/

	if (Class)
	{
		Class->SerializeScriptProperties(Ar, This());
	}
}