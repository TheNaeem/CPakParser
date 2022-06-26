#include "CoreTypes.h"
#include "Archives.h"

FArchiveState::FArchiveState()
{
	SerializedPropertyChain = nullptr;
	Reset();
}

FArchiveState::~FArchiveState()
{
	delete CustomVersionContainer;
	delete SerializedPropertyChain;
}

void FArchiveState::Reset()
{
	ArIsLoading = false;
	ArIsLoadingFromCookedPackage = false;
	ArIsSaving = false;
	ArIsTransacting = false;
	ArIsTextFormat = false;
	ArWantBinaryPropertySerialization = false;
	ArUseUnversionedPropertySerialization = false;
	ArForceUnicode = false;
	ArIsPersistent = false;
	ArIsError = false;
	ArIsCriticalError = false;
	ArContainsCode = false;
	ArContainsMap = false;
	ArRequiresLocalizationGather = false;
	ArForceByteSwapping = false;
	ArSerializingDefaults = false;
	ArIgnoreArchetypeRef = false;
	ArNoDelta = false;
	ArNoIntraPropertyDelta = false;
	ArIgnoreOuterRef = false;
	ArIgnoreClassGeneratedByRef = false;
	ArIgnoreClassRef = false;
	ArAllowLazyLoading = false;
	ArIsObjectReferenceCollector = false;
	ArIsModifyingWeakAndStrongReferences = false;
	ArIsCountingMemory = false;
	ArPortFlags = 0;
	ArShouldSkipBulkData = false;
	ArShouldSkipCompilingAssets = false;
	ArMaxSerializeSize = 0;
	ArIsFilterEditorOnly = false;
	ArIsSaveGame = false;
	ArIsNetArchive = false;
	ArUseCustomPropertyList = false;
	bCustomVersionsAreReset = true;
	SerializedProperty = nullptr;

	delete SerializedPropertyChain;
	SerializedPropertyChain = nullptr;
}