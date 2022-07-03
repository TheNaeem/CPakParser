#pragma once

#include "Archives.h"

class FMemoryReader : public FMemoryArchive
{
public:
	virtual std::string GetArchiveName() const override
	{
		return "FMemoryReader";
	}

	virtual int64_t TotalSize() override
	{
		return std::min((int64_t)Bytes.size(), LimitSize);
	}

	void Serialize(void* Data, uint64_t Num) override
	{
		if (Num)
		{
			if (Offset + Num <= TotalSize())
			{
				memcpy(Data, &Bytes[Offset], Num);
				Offset += Num;
			}
		}
	}

	explicit FMemoryReader(const std::vector<uint8_t>& InBytes)
		: Bytes(InBytes)
		, LimitSize(INT64_MAX)
	{
	}

	FMemoryReader(uint8_t* InBytes, uint32_t Size)
		: Bytes(std::vector<uint8_t>(InBytes, InBytes + Size))
		, LimitSize(INT64_MAX)
	{
	}

	void SetLimitSize(int64_t NewLimitSize)
	{
		LimitSize = NewLimitSize;
	}

private:
	const std::vector<uint8_t>& Bytes;
	int64_t LimitSize;
};

class FMemoryReaderView : public FMemoryArchive
{
public:
	virtual std::string GetArchiveName() const override
	{
		return "FMemoryReaderView";
	}

	virtual int64_t TotalSize() override
	{
		return std::min(static_cast<int64_t>(Bytes.size()), LimitSize);
	}

	void Serialize(void* Data, uint64_t Num) override
	{
		if (Offset + Num <= TotalSize())
		{
			memcpy(Data, Bytes.data() + Offset, Num);
			Offset += Num;
		}
	}

	explicit FMemoryReaderView(std::span<uint8_t> InBytes, bool bIsPersistent = false)
		: Bytes(InBytes)
		, LimitSize(INT64_MAX)
	{
	}

	void SetLimitSize(int64_t NewLimitSize)
	{
		LimitSize = NewLimitSize;
	}

private:
	std::span<uint8_t> Bytes;
	int64_t LimitSize;
};