#pragma once

#include <cstdint>

class FIoContainerId
{
public:

	FIoContainerId();

	inline FIoContainerId(const FIoContainerId& Other) = default;
	inline FIoContainerId(FIoContainerId&& Other) = default;
	inline FIoContainerId& operator=(const FIoContainerId& Other) = default;

	uint64_t Value() const;
	bool IsValid() const;
	bool operator<(FIoContainerId& Other) const;
	bool operator==(FIoContainerId& Other) const;
	bool operator!=(FIoContainerId& Other) const;
	friend uint32_t hash_value(const FIoContainerId& In);

	friend class FArchive& operator<<(FArchive& Ar, FIoContainerId& ContainerId);

private:
	inline explicit FIoContainerId(const uint64_t InId)
		: Id(InId) { }

	static constexpr uint64_t InvalidId = uint64_t(-1);

	uint64_t Id;
};