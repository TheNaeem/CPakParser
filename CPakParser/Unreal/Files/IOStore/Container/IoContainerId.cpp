#include "IoContainerId.h"

#include "Serialization/Archives.h"

FIoContainerId::FIoContainerId() : Id(InvalidId)
{
}

uint64_t FIoContainerId::Value() const
{
	return Id;
}

bool FIoContainerId::IsValid() const
{
	return Id != InvalidId;
}

bool FIoContainerId::operator<(FIoContainerId& Other) const
{
	return Id < Other.Id;
}

bool FIoContainerId::operator==(FIoContainerId& Other) const
{
	return Id == Other.Id;
}

bool FIoContainerId::operator!=(FIoContainerId& Other) const
{
	return Id != Other.Id;
}

uint32_t hash_value(const FIoContainerId& In)
{
	return uint32_t(In.Id);
}

FArchive& operator<<(FArchive& Ar, FIoContainerId& ContainerId)
{
	Ar << ContainerId.Id;

	return Ar;
}