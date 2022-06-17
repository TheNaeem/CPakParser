#include "CoreTypes.h"

#include <filesystem>

FAESKey::FAESKey(std::string KeyHexString)
{
	for (unsigned int i = 0; i < KeyHexString.length() / 2; i++)
	{
		Key[i] = static_cast<uint8_t>(strtol(KeyHexString.substr(i * 2, 2).c_str(), NULL, 16));
	}
}

uint32_t ConvertHexStringToInt(std::string hexString)
{
	uint32_t ret;
	std::stringstream ss;
	ss << std::hex << hexString.c_str();
	ss >> ret;

	return ret;
}

bool FAESKey::IsValid()
{
	auto Words = (uint32_t*)Key;
	for (auto Index = 0; Index < KeySize / 4; ++Index)
	{
		if (Words[Index] != 0)
		{
			return true;
		}
	}

	return false;
}

std::string FAESKey::ToString()
{
	std::ostringstream ret;
	ret << std::hex << std::uppercase << std::setfill('0');

	for (size_t i = 0; i < KeySize; i++)
	{
		ret << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(Key[i]);
	}

	return ret.str();
}

FGuid::FGuid(std::string HexString)
{
	A = ConvertHexStringToInt(HexString.substr(0, 8));
	B = ConvertHexStringToInt(HexString.substr(8, 8));
	C = ConvertHexStringToInt(HexString.substr(16, 8));
	D = ConvertHexStringToInt(HexString.substr(24, 8));
}