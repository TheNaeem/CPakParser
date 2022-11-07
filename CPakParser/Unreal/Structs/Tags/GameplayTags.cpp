#include "GameplayTagContainer.h"

#include "Serialization/Archives.h"
#include "Logger.h"

void operator<<(FArchive& Ar, FGameplayTag& GameplayTag)
{
	Ar << GameplayTag.TagName;
}

void operator<<(FArchive& Ar, FGameplayTagContainer& GameplayTagContainer)
{
	if (Ar.UEVer() < VER_UE4_GAMEPLAY_TAG_CONTAINER_TAG_TYPE_CHANGE)
	{
		std::vector<FName> Tags_DEPRECATED;
		Ar << Tags_DEPRECATED;
		Log<Error>("Failed to load old GameplayTag container, too old to migrate correctly");
	}
	else Ar << GameplayTagContainer.GameplayTags;
}