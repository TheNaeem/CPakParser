#include "Name.h"

#include "Serialization/Archives.h"
#include <span>

struct FSerializedNameHeader
{
	FSerializedNameHeader() {}

	FSerializedNameHeader(uint32_t Len, bool bIsUtf16)
	{
		Data[0] = uint8_t(bIsUtf16) << 7 | static_cast<uint8_t>(Len >> 8);
		Data[1] = static_cast<uint8_t>(Len);
	}

	uint8_t IsUtf16() const
	{
		return Data[0] & 0x80u;
	}

	uint32_t Len() const
	{
		return ((Data[0] & 0x7Fu) << 8) + Data[1];
	}

	uint32_t NumBytes() const
	{
		return IsUtf16() ? sizeof(wchar_t) * Len() : sizeof(char) * Len();
	}

	uint8_t Data[2];
};

struct FNameBatchLoader
{
	std::span<const FSerializedNameHeader> Headers;
	std::span<const uint8_t> Strings;
	std::vector<uint8_t> Data;

	bool Read(FArchive& Ar)
	{
		uint32_t Num = 0;
		Ar << Num;

		if (!Num) return false;

		uint32_t NumStringBytes = 0;
		Ar << NumStringBytes;

		Ar.Seek(Ar.Tell() + sizeof(uint64_t)); //dont need the hash version

		uint32_t NumHashBytes = sizeof(uint64_t) * Num;
		uint32_t NumHeaderBytes = sizeof(FSerializedNameHeader) * Num;

		Data.resize(NumHashBytes + NumHeaderBytes + NumStringBytes, '\0');
		Ar.Serialize(Data.data(), Data.size());

		Headers = std::span<FSerializedNameHeader>{ reinterpret_cast<FSerializedNameHeader*>(Data.data() + NumHashBytes), Num };
		Strings = std::span<uint8_t>{ Data.data() + NumHashBytes + NumHeaderBytes, NumStringBytes };

		return true;
	}

	std::vector<std::string> Load()
	{
		std::vector<std::string> Ret(Headers.size());

		uint32_t NameOffset = 0;
		for (auto&& Header : Headers)
		{
			union
			{
				const uint8_t* Data;
				const char* AnsiString;
				const wchar_t* WideString;
			};

			Data = Strings.data() + NameOffset;

			std::string NameString;

			if (Header.IsUtf16())
			{
				std::wstring Temp = std::wstring(WideString, Header.Len());
				NameString.assign(Temp.begin(), Temp.end());
			}
			else
			{
				NameString.assign(AnsiString, Header.Len());
			}

			Ret.push_back(NameString);
			NameOffset += Header.IsUtf16() ? sizeof(wchar_t) * Header.Len() : sizeof(char) * Header.Len();
		}

		return Ret;
	}
};

std::vector<std::string> LoadNameBatch(FArchive& Ar)
{
	FNameBatchLoader Loader;

	if (!Loader.Read(Ar))
		return std::vector<std::string>();

	return Loader.Load();
}