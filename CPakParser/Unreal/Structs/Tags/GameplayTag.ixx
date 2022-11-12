export module GameplayTag;

export import Name;
import FArchiveBase;

export class FGameplayTag
{
public:

	friend void operator<<(FArchive& Ar, FGameplayTag& GameplayTag)
	{
		Ar << GameplayTag.TagName;
	}

	__forceinline std::string ToString() const
	{
		return TagName.ToString();
	}

	__forceinline FName GetName() const
	{
		return TagName;
	}

private:

	FName TagName;
};