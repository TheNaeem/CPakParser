export module CPakParser.Structs.GameplayTagContainer;

export import CPakParser.Structs.GameplayTag;
import <vector>;
import CPakParser.Serialization.FArchive;
import CPakParser.Logging;

export class FGameplayTagContainer
{
public:

	FGameplayTagContainer()
	{
	}

	__forceinline int32_t Num() const
	{
		return GameplayTags.size();
	}

	__forceinline bool IsValid() const
	{
		return GameplayTags.size() > 0;
	}

	__forceinline bool IsEmpty() const
	{
		return GameplayTags.size() == 0;
	}

	bool IsValidIndex(int32_t Index) const
	{
		return Index < GameplayTags.size();
	}

	FGameplayTag GetByIndex(int32_t Index) const
	{
		if (IsValidIndex(Index))
		{
			return GameplayTags[Index];
		}
		return FGameplayTag();
	}

	FGameplayTag First() const
	{
		return GameplayTags.size() > 0 ? GameplayTags[0] : FGameplayTag();
	}

	FGameplayTag Last() const
	{
		return GameplayTags.size() > 0 ? GameplayTags.back() : FGameplayTag();
	}

	friend void operator<<(FArchive& Ar, FGameplayTagContainer& GameplayTagContainer)
	{
		if (Ar.UEVer() < VER_UE4_GAMEPLAY_TAG_CONTAINER_TAG_TYPE_CHANGE)
		{
			std::vector<FName> Tags_DEPRECATED;
			Ar << Tags_DEPRECATED;
			LogError("Failed to load old GameplayTag container, too old to migrate correctly");
		}
		else Ar << GameplayTagContainer.GameplayTags;
	}

private:

	std::vector<FGameplayTag> GameplayTags;
};