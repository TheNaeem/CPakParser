#pragma once

#include "Archives.h"
#include <span>

class FMemoryArchive : public FArchive
{
public:
	virtual std::string GetArchiveName() const { return "FMemoryArchive"; }

	void Seek(int64_t InPos) final
	{
		Offset = InPos;
	}

	int64_t Tell() final
	{
		return Offset;
	}

	using FArchive::operator<<;

	virtual FArchive& operator<<(class FName& N) override
	{
		std::string StringName;
		*this << StringName;
		N = FName(StringName);

		return *this;
	}

	virtual FArchive& operator<<(class UObject*& Res) override
	{
		return *this;
	}

protected:
	FMemoryArchive()
		: FArchive(), Offset(0)
	{
	}

	int64_t	Offset;
};

class FMemoryReader : public FMemoryArchive
{
public:

	virtual std::string GetArchiveName() const override
	{
		return "FMemoryReader";
	}

	virtual int64_t TotalSize() override
	{
		if (LimitSize < Bytes.size())
			return LimitSize;

		return Bytes.size();
	}

	void Serialize(void* Data, int64_t Num) override
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

	__forceinline const uint8_t* GetBuffer() const
	{
		return Bytes.data();
	}

	__forceinline const uint8_t* GetBufferCur() const
	{
		return Bytes.data() + Offset;
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
		if (LimitSize < Bytes.size())
			return LimitSize;

		return Bytes.size();
	}

	void Serialize(void* Data, int64_t Num) override
	{
		if (Offset + Num <= TotalSize())
		{
			memcpy(Data, Bytes.data() + Offset, Num);
			Offset += Num;
		}
	}

	FMemoryReaderView(std::vector<uint8_t>& InBytes)
		: Bytes(std::span<uint8_t>{InBytes.data(), InBytes.size()})
		, LimitSize(INT64_MAX)
	{
	}

	FMemoryReaderView(std::span<uint8_t> InBytes)
		: Bytes(InBytes)
		, LimitSize(INT64_MAX)
	{
	}

	void SetLimitSize(int64_t NewLimitSize)
	{
		LimitSize = NewLimitSize;
	}

	__forceinline const uint8_t* GetBuffer() const
	{
		return Bytes.data();
	}

	__forceinline const uint8_t* GetBufferCur() const
	{
		return Bytes.data() + Offset;
	}

private:
	std::span<uint8_t> Bytes;
	int64_t LimitSize;
};