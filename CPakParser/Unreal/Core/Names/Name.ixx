export module CPakParser.Core.FName;

export import <string>;
import <vector>;
import CPakParser.Serialization.FArchive;

export class FName
{
	std::string Val;

public:

	friend class FNameProperty;

	FName() = default;

	FName(std::string& InStr) : Val(InStr)
	{
	}

	FName(std::string_view& StrView) : Val(StrView)
	{
	}

	__forceinline void operator=(std::string const& Other)
	{
		Val = Other;
	}

	__forceinline void operator=(FName const& Other) 
	{
		Val = Other.Val;
	}

	bool operator==(FName const& Other) const
	{
		return Val == Other.Val;
	}

	bool operator!=(const FName& Other) const
	{
		return !(*this == Other);
	}

	__forceinline std::string ToString() const
	{
		return Val;
	}

	__forceinline std::string& GetString()
	{
		return Val;
	}
};

export std::vector<std::string> LoadNameBatch(FArchive& Ar);