#include "Class.h"

#include "Serialization/Impl/ExportReader.h"

void UObject::Serialize(TSharedPtr<FExportReader> Ar)
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