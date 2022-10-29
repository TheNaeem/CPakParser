#pragma once

#include "../Class.h"

class FPropertyIterator
{
	static constexpr int Invalid = -1;

public:

	__forceinline FPropertyIterator(UStructPtr Struct) : CurrentStruct(Struct)
	{
		for (auto Super = Struct; Super; Super = Super->GetSuper())
			StructChain.push_back(Super);

		StructIndex = StructChain.size() - 1;
		CurrentStruct = StructChain[StructIndex];
	}

	void Next()
	{
		PropIndex++;

		if (PropIndex >= CurrentStruct->GetProperties().size())
		{
			if (!StructIndex) // we've finished iterating the properties
			{
				PropIndex = Invalid;
				return;
			}

			StructIndex--;
			PropIndex = 0;
			CurrentStruct = StructChain[StructIndex];
		}
	}

	__forceinline void operator++()
	{
		Next();
	}

	__forceinline operator bool() const
	{
		return PropIndex != Invalid;
	}

	__forceinline FProperty* operator*()
	{
		while (!CurrentStruct->GetProperties().size())
			Next();

		return CurrentStruct->GetProperties()[PropIndex];
	}

private:

	std::vector<UStructPtr> StructChain;
	UStructPtr& CurrentStruct;

	int PropIndex = 0;
	int StructIndex = 0;
};