#pragma once

#include <string>
#include <vector>

class FName
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

std::vector<std::string> LoadNameBatch(class FArchive& Ar);