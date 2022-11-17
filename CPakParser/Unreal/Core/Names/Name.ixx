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

	__forceinline std::string ToString() const
	{
		return Val;
	}
};

export std::vector<std::string> LoadNameBatch(FArchive& Ar);