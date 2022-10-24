#pragma once

#include <cstdint>

struct FIoOffsetAndLength
{
public:

	FIoOffsetAndLength();

	uint64_t GetOffset() const;
	uint64_t GetLength() const;
	void SetOffset(uint64_t Offset);
	void SetLength(uint64_t Length);
	bool IsValid();

private:

	uint8_t OffsetAndLength[5 + 5];
};
