#include "CoreTypes.h"
#include "Archives.h"

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