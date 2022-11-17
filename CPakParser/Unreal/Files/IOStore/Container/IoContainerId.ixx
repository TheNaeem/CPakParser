export module CPakParser.IOStore.ContainerId;

import CPakParser.Serialization.FArchive;
import <cstdint>;

export class FIoContainerId
{
public:

	FIoContainerId() : Id(-1)
	{
	}

	inline FIoContainerId(const FIoContainerId& Other) = default;
	inline FIoContainerId(FIoContainerId&& Other) = default;
	inline FIoContainerId& operator=(const FIoContainerId& Other) = default;

	uint64_t Value() const
	{
		return Id;
	}

	bool IsValid() const
	{
		return Id != -1;
	}

	bool operator<(FIoContainerId& Other) const
	{
		return Id < Other.Id;
	}

	bool operator==(FIoContainerId& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(FIoContainerId& Other) const
	{
		return Id != Other.Id;
	}

	friend uint32_t hash_value(const FIoContainerId& In)
	{
		return uint32_t(In.Id);
	}

	friend FArchive& operator<<(FArchive& Ar, FIoContainerId& ContainerId)
	{
		return Ar << ContainerId.Id;
	}

private:
	inline explicit FIoContainerId(const uint64_t InId)
		: Id(InId) { }

	uint64_t Id;
};