#include "Class.h"

#include "ExportReader.h"

std::string UObject::GetName()
{
	return Name;
}

UClassPtr UObject::GetClass()
{
	return Class;
}

UObjectPtr UObject::GetOuter()
{
	return Outer;
}

void UObject::SetName(std::string& Val) 
{
	Name = Val;
}

void UObject::SetClass(UClassPtr Val)
{
	Class = Val;
}

void UObject::SetOuter(UObjectPtr Val)
{
	Outer = Val;
}

bool UObject::IsLoaded()
{
	return Flags & RF_WasLoaded;
}

void UObject::Copy(UObjectPtr Other)
{
	Class = Other->Class;
	Outer = Other->Outer;
	Name = Other->Name;
	Flags = Other->Flags;
}

void UObject::SerializeScriptProperties(TSharedPtr<FExportReader> Ar)
{

}

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
		Class->SerializeScriptProperties(Ar); 
	}
}