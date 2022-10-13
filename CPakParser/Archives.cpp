#include "Archives.h"

#include "Class.h"

FArchiveState::FArchiveState()
{
	SerializedPropertyChain = nullptr;
	Reset();
}

FArchiveState::~FArchiveState()
{
	if (CustomVersionContainer)
		delete CustomVersionContainer;

	if (SerializedPropertyChain)
		delete SerializedPropertyChain;
}

void FArchiveState::Reset()
{
	bUseUnversionedProperties = false;
	ArSerializingDefaults = false;
	ArPortFlags = 0;
	ArMaxSerializeSize = 0;
	bCustomVersionsAreReset = true;
	SerializedProperty = nullptr;

	delete SerializedPropertyChain;
	SerializedPropertyChain = nullptr;
}

void FArchive::Preload(UObject* Obj) // TODO: 
{
	auto Struct = dynamic_cast<UStruct*>(Obj);

	if (!Struct)
		return;

	if (Struct->GetSuper())
	{
		Preload(Struct->GetSuper().Get()); // TODO: make this loadable by the uobject ptr
	}

	if (!Struct->HasAnyFlags(RF_NeedLoad))
		return;
}