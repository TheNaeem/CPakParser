#pragma once

#include "CoreTypes.h"
#include "Versioning.h"

struct FArchiveSerializedPropertyChain
{
public:
	/** Default constructor */
	FArchiveSerializedPropertyChain()
		: SerializedPropertyChainUpdateCount(0)
	{
	}

	void PushProperty(class FProperty* InProperty, const bool bIsEditorOnlyProperty)
	{
		SerializedPropertyChain.push_back(InProperty);
		IncrementUpdateCount();
	}

	void PopProperty(class FProperty* InProperty)
	{
		SerializedPropertyChain.pop_back();
		IncrementUpdateCount();
	}

	class FProperty* GetPropertyFromStack(const int32_t InStackIndex) const
	{
		return SerializedPropertyChain.rbegin()[InStackIndex];
	}

	class FProperty* GetPropertyFromRoot(const int32_t InRootIndex) const
	{
		return SerializedPropertyChain[InRootIndex];
	}

	uint32_t GetNumProperties() const
	{
		return SerializedPropertyChain.size();
	}

	uint32_t GetUpdateCount() const
	{
		return SerializedPropertyChainUpdateCount;
	}

private:
	void IncrementUpdateCount()
	{
		while (++SerializedPropertyChainUpdateCount == 0) {} 
	}

	std::vector<class FProperty*> SerializedPropertyChain;
	uint32_t SerializedPropertyChainUpdateCount;
};

struct FArchiveState
{
protected:
	uint8_t ArIsLoading : 1;
	uint8_t ArIsLoadingFromCookedPackage : 1;
	uint8_t ArIsSaving : 1;
	uint8_t ArIsTransacting : 1;
	uint8_t ArIsTextFormat : 1;
	uint8_t ArWantBinaryPropertySerialization : 1;
	uint8_t ArUseUnversionedPropertySerialization : 1;
	uint8_t ArForceUnicode : 1;
	uint8_t ArIsPersistent : 1;

private:
	uint8_t ArIsError : 1;
	uint8_t ArIsCriticalError : 1;
	uint8_t ArShouldSkipCompilingAssets : 1;

public:
	uint8_t ArContainsCode : 1;
	uint8_t ArContainsMap : 1;
	uint8_t ArRequiresLocalizationGather : 1;
	uint8_t ArForceByteSwapping : 1;
	uint8_t ArIgnoreArchetypeRef : 1;
	uint8_t ArNoDelta : 1;
	uint8_t ArNoIntraPropertyDelta : 1;
	uint8_t ArIgnoreOuterRef : 1;
	uint8_t ArIgnoreClassGeneratedByRef : 1;
	uint8_t ArIgnoreClassRef : 1;
	uint8_t ArAllowLazyLoading : 1;
	uint8_t ArIsObjectReferenceCollector : 1;
	uint8_t ArIsModifyingWeakAndStrongReferences : 1;
	uint8_t ArIsCountingMemory : 1;
	uint8_t ArShouldSkipBulkData : 1;
	uint8_t ArIsFilterEditorOnly : 1;
	uint8_t ArIsSaveGame : 1;
	uint8_t ArIsNetArchive : 1;
	uint8_t ArUseCustomPropertyList : 1;
	int32_t ArSerializingDefaults;
	uint32_t ArPortFlags;
	int64_t ArMaxSerializeSize;

protected:
	FPackageFileVersion ArUEVer;
	int32_t ArLicenseeUEVer;
	FEngineVersionBase ArEngineVer;
	uint32_t ArEngineNetVer;
	uint32_t ArGameNetVer;
	mutable FCustomVersionContainer* CustomVersionContainer = nullptr;
	FProperty* SerializedProperty;
	FArchiveSerializedPropertyChain* SerializedPropertyChain;
	mutable bool bCustomVersionsAreReset;

private:
	FArchiveState* NextProxy = nullptr;
};

class FArchive : private FArchiveState
{
public:
	FArchive() = default;
	FArchive(const FArchive&) = default;
	FArchive& operator=(const FArchive& ArchiveToCopy) = default;
	~FArchive() = default;

	virtual FArchive& operator<<(FName& Value)
	{
		return *this;
	}

};