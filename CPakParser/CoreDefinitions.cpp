#include "CoreTypes.h"

#include "Archives.h"
#include "CityHash.h"
#include <filesystem>
#include <span>

uint32_t ConvertHexStringToInt(std::string hexString)
{
	uint32_t ret;
	std::stringstream ss;
	ss << std::hex << hexString.c_str();
	ss >> ret;

	return ret;
}

FGuid::FGuid(std::string HexString)
{
	A = ConvertHexStringToInt(HexString.substr(0, 8));
	B = ConvertHexStringToInt(HexString.substr(8, 8));
	C = ConvertHexStringToInt(HexString.substr(16, 8));
	D = ConvertHexStringToInt(HexString.substr(24, 8));
}

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
		for (auto i = 0; i < Headers.size(); i++)
		{
			auto Header = Headers[i];

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

			Ret[i] = NameString;
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

void FEncryptionKeyManager::AddKey(const FGuid& InGuid, const FAESKey InKey)
{
	SCOPE_LOCK(CriticalSection);

	if (!Keys.contains(InGuid))
	{
		Keys.insert_or_assign(InGuid, InKey);
	}
}

bool FEncryptionKeyManager::GetKey(const FGuid& InGuid, FAESKey& OutKey)
{
	SCOPE_LOCK(CriticalSection);

	if (!Keys.contains(InGuid)) return false;

	OutKey = Keys[InGuid];
	return true;
}

bool const FEncryptionKeyManager::HasKey(const FGuid& InGuid)
{
	return Keys.contains(InGuid);
}

const phmap::flat_hash_map<FGuid, FAESKey>& FEncryptionKeyManager::GetKeys()
{
	return Keys;
}

void FNameMap::Serialize(class FArchive& Ar, FMappedName::EType NameMapType)
{
	NameEntries = LoadNameBatch(Ar);
	this->NameMapType = NameMapType;
}

FPackageId::FPackageId(std::string Name)
{
	std::wstring NameW(Name.begin(), Name.end());

	std::transform(NameW.begin(), NameW.end(), NameW.begin(),
		[](unsigned char c) { return std::tolower(c); });

	Id = CityHash64(reinterpret_cast<const char*>(NameW.data()), NameW.size() * 2 /* size of bytes */);
}