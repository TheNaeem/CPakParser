export module CPakParser.Misc.FGuid;

import <string>;
import <sstream>;
import CPakParser.Serialization.FArchive;

uint32_t ConvertHexStringToInt(std::string hexString)
{
	uint32_t ret;
	std::stringstream ss;
	ss << std::hex << hexString.c_str();
	ss >> ret;

	return ret;
}

export struct FGuid
{
	int32_t A, B, C, D;

	constexpr FGuid(int32_t InA, int32_t InB, int32_t InC, int32_t InD)
		: A(InA), B(InB), C(InC), D(InD)
	{ }

	FGuid() :
		A(0),
		B(0),
		C(0),
		D(0)
	{
	}

	FGuid(std::string HexString)
	{
		A = ConvertHexStringToInt(HexString.substr(0, 8));
		B = ConvertHexStringToInt(HexString.substr(8, 8));
		C = ConvertHexStringToInt(HexString.substr(16, 8));
		D = ConvertHexStringToInt(HexString.substr(24, 8));
	}

	friend FArchive& operator<<(FArchive& Ar, FGuid& Value)
	{
		Ar.Serialize(&Value.A, sizeof(FGuid));
		return Ar;
	}

	bool operator==(FGuid Other) const
	{
		return Other.A == A && Other.B == B && Other.C == C && Other.D == D;
	}

	bool operator!=(FGuid Other) const
	{
		return !(*this == Other);
	}

	bool IsValid() const
	{
		return ((A | B | C | D) != 0);
	}

	void Invalidate()
	{
		A = B = C = D = 0;
	}

	friend size_t hash_value(const FGuid& Guid)
	{
		return
			std::hash<int32_t>{}(Guid.A) ^
			std::hash<int32_t>{}(Guid.B) ^
			std::hash<int32_t>{}(Guid.C) ^
			std::hash<int32_t>{}(Guid.D);
	}
};