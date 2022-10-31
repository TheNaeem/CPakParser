#pragma once

#include <string>
#include <vector>

class FName
{
	std::string Val;

public:

	FName() = default;

	FName(std::string& InStr) : Val(InStr)
	{
	}

	__forceinline std::string ToString() const
	{
		return Val;
	}
};

std::vector<std::string> LoadNameBatch(class FArchive& Ar);