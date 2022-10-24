#include "Guid.h"

#include "Serialization/Archives.h"
#include <sstream>

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

size_t hash_value(const FGuid& Guid)
{
	return
		std::hash<int32_t>{}(Guid.A) ^
		std::hash<int32_t>{}(Guid.B) ^
		std::hash<int32_t>{}(Guid.C) ^
		std::hash<int32_t>{}(Guid.D);
}

FArchive& operator<<(FArchive& Ar, FGuid& Value)
{
	Ar.Serialize(&Value.A, sizeof(FGuid));
	return Ar;
}