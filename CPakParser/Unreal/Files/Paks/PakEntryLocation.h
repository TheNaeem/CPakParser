#pragma once

#include "../FileEntry.h"

struct FPakEntryLocation : public FFileEntryInfo
{
	static const int32_t Invalid = MIN_int32;
	static const int32_t MaxIndex = MAX_int32 - 1;

	FPakEntryLocation() : FFileEntryInfo(Invalid)
	{
	}

	friend size_t hash_value(const FPakEntryLocation& in)
	{
		return (in.Entry.PakIndex * 0xdeece66d + 0xb);
	}

	__forceinline friend bool operator==(const FPakEntryLocation& entry1, const FPakEntryLocation& entry2)
	{
		return entry1.Entry.PakIndex == entry2.Entry.PakIndex;
	}

	bool IsInvalid() const
	{
		return Entry.PakIndex <= Invalid || MaxIndex < Entry.PakIndex;
	}

	bool IsOffsetIntoEncoded() const
	{
		return 0 <= Entry.PakIndex && Entry.PakIndex <= MaxIndex;
	}

	bool IsListIndex() const
	{
		return (-MaxIndex - 1) <= Entry.PakIndex && Entry.PakIndex <= -1;
	}

	int32_t GetAsOffsetIntoEncoded() const
	{
		return IsOffsetIntoEncoded() ? Entry.PakIndex : -1;
	}

	int32_t GetAsListIndex() const
	{
		if (IsListIndex())
		{
			return -(Entry.PakIndex + 1);
		}

		return -1;
	}

private:

	explicit FPakEntryLocation(int32_t InIndex)
	{
		Entry.PakIndex = InIndex;
	}
};