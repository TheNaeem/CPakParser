#include "IoOffsetAndLength.h"

#include <vcruntime_string.h>

FIoOffsetAndLength::FIoOffsetAndLength()
{
	memset(&OffsetAndLength[0], 0, sizeof(OffsetAndLength));
}

uint64_t FIoOffsetAndLength::GetOffset() const
{
	return OffsetAndLength[4]
		| (uint64_t(OffsetAndLength[3]) << 8)
		| (uint64_t(OffsetAndLength[2]) << 16)
		| (uint64_t(OffsetAndLength[1]) << 24)
		| (uint64_t(OffsetAndLength[0]) << 32);
}

uint64_t FIoOffsetAndLength::GetLength() const
{
	return OffsetAndLength[9]
		| (uint64_t(OffsetAndLength[8]) << 8)
		| (uint64_t(OffsetAndLength[7]) << 16)
		| (uint64_t(OffsetAndLength[6]) << 24)
		| (uint64_t(OffsetAndLength[5]) << 32);
}

void FIoOffsetAndLength::SetOffset(uint64_t Offset)
{
	OffsetAndLength[0] = uint8_t(Offset >> 32);
	OffsetAndLength[1] = uint8_t(Offset >> 24);
	OffsetAndLength[2] = uint8_t(Offset >> 16);
	OffsetAndLength[3] = uint8_t(Offset >> 8);
	OffsetAndLength[4] = uint8_t(Offset >> 0);
}

void FIoOffsetAndLength::SetLength(uint64_t Length)
{
	OffsetAndLength[5] = uint8_t(Length >> 32);
	OffsetAndLength[6] = uint8_t(Length >> 24);
	OffsetAndLength[7] = uint8_t(Length >> 16);
	OffsetAndLength[8] = uint8_t(Length >> 8);
	OffsetAndLength[9] = uint8_t(Length >> 0);
}

bool FIoOffsetAndLength::IsValid()
{
	for (size_t i = 0; i < 10; i++)
	{
		if (OffsetAndLength[i] != 0) return true;
	}

	return false;
}