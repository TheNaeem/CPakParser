export module CPakParser.Structs.GameplayTag;

export import CPakParser.Core.FName;
import CPakParser.Serialization.FArchive;

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