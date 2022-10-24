#include "ExportReader.h"

#include "Core/Class.h"

void FExportReader::Preload(UObjectPtr Obj) // TODO: 
{
	auto Struct = Obj.As<UStruct>();

	if (!Struct)
		return;

	if (Struct->GetSuper())
	{
		Preload(Struct->GetSuper()); // TODO: make this loadable by the uobject ptr
	}

	if (!Struct->HasAnyFlags(UObject::Flags::RF_NeedLoad))
		return;
}