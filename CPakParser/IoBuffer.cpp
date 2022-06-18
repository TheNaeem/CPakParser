#include "CoreTypes.h"
#include "IOStore.h"
#include <windows.h>

uint32_t FIoBuffer::BufCore::AddRef() const
{
	return uint32_t(InterlockedIncrement(reinterpret_cast<long*>(&NumRefs)));
}

uint32_t FIoBuffer::BufCore::Release() const
{
	const int32_t Refs = InterlockedDecrement(reinterpret_cast<long*>(&NumRefs));

	if (Refs == 0)
	{
		delete this;
	}

	return uint32_t(Refs);
}

void FIoBuffer::BufCore::SetDataAndSize(const uint8_t* InData, uint64_t InSize)
{
	DataPtr = const_cast<uint8_t*>(InData);
	DataSizeLow = uint32_t(InSize & 0xffffffffu);
	DataSizeHigh = (InSize >> 32) & 0xffu;
}

FIoBuffer::BufCore::~BufCore()
{
	if (IsMemoryOwned())
	{
		free(Data());
	}
}

FIoBuffer::BufCore::BufCore(uint64_t InSize)
{
	auto NewBuffer = reinterpret_cast<uint8_t*>(malloc(InSize));
	SetDataAndSize(NewBuffer, InSize);
	SetIsOwned(true);
}

FIoBuffer::FIoBuffer() : CorePtr(new BufCore)
{
}

FIoBuffer::FIoBuffer(uint64_t InSize) : CorePtr(new BufCore(InSize))
{
}