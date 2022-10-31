#pragma once

#include "../Class.h"

class FPropertyIterator
{
	static constexpr int Invalid = -1;

public:

	__forceinline FPropertyIterator(UStructPtr InStruct) : Struct(InStruct)
	{
	}

	void Next()
	{
		PropIndex++;

		if (PropIndex >= Struct->GetProperties().size())
		{
			PropIndex = 0;
			Struct = Struct->GetSuper();
		}
	}

	__forceinline void operator++()
	{
		Next();
	}

	__forceinline void operator+=(int Num)
	{
		while (Num--)
			Next();
	}

	__forceinline operator bool() const
	{
		return Struct;
	}

	__forceinline FProperty* operator*()
	{
		while (!Struct->GetProperties().size())
			Next();

		return Struct->GetProperties()[PropIndex];
	}

private:

	UStructPtr Struct;

	int PropIndex = 0;
};