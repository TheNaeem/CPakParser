export module CPakParser.Package.Id;

import <string>;
import CPakParser.Serialization.FArchive;

export class FPackageId
{
	static constexpr uint64_t InvalidId = 0;
	uint64_t Id = InvalidId;

	inline explicit FPackageId(uint64_t InId) : Id(InId) {}

public:

	FPackageId() = default;
	FPackageId(std::string Name);

	static FPackageId FromValue(const uint64_t Value)
	{
		return FPackageId(Value);
	}

	__forceinline bool IsValid() const
	{
		return Id != InvalidId;
	}

	__forceinline uint64_t Value() const
	{
		return Id;
	}

	__forceinline uint64_t ValueForDebugging() const
	{
		return Id;
	}

	__forceinline bool operator<(FPackageId Other) const
	{
		return Id < Other.Id;
	}

	__forceinline bool operator==(FPackageId Other) const
	{
		return Id == Other.Id;
	}

	__forceinline bool operator!=(FPackageId Other) const
	{
		return Id != Other.Id;
	}

	__forceinline friend size_t hash_value(const FPackageId& In)
	{
		return In.Id;
	}

	friend FArchive& operator<<(FArchive& Ar, FPackageId& Value)
	{
		Ar << Value.Id;

		return Ar;
	}
};