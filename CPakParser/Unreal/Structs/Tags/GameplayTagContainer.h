#pragma once

#include "GameplayTag.h"

class FGameplayTagContainer
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

	friend void operator<<(class FArchive& Ar, FGameplayTagContainer& GameplayTagContainer);

private:

	std::vector<FGameplayTag> GameplayTags;
};