#include "CoreTypes.h"

#include <filesystem>

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