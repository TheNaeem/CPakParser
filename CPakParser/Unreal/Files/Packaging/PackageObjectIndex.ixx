export module CPakParser.Package.ObjectIndex;

import <cstdint>;

export class FPackageObjectIndex
{
	static constexpr uint64_t IndexBits = 62ull;
	static constexpr uint64_t IndexMask = (1ull << IndexBits) - 1ull;
	static constexpr uint64_t TypeMask = ~IndexMask;
	static constexpr uint64_t TypeShift = IndexBits;
	static constexpr uint64_t Invalid = ~0ull;

	uint64_t TypeAndId = Invalid;

	enum EType
	{
		Export,
		ScriptImport,
		PackageImport,
		Null,
		TypeCount = Null,
	};

	__forceinline explicit FPackageObjectIndex(EType InType, uint64_t InId) : TypeAndId((uint64_t(InType) << TypeShift) | InId) {}

public:

	FPackageObjectIndex() = default;

	__forceinline static FPackageObjectIndex FromExportIndex(const int32_t Index)
	{
		return FPackageObjectIndex(Export, Index);
	}

	__forceinline bool IsNull() const
	{
		return TypeAndId == Invalid;
	}

	__forceinline bool IsExport() const
	{
		return (TypeAndId >> TypeShift) == Export;
	}

	__forceinline bool IsImport() const
	{
		return IsScriptImport() || IsPackageImport();
	}

	__forceinline bool IsScriptImport() const
	{
		return (TypeAndId >> TypeShift) == ScriptImport;
	}

	__forceinline bool IsPackageImport() const
	{
		return (TypeAndId >> TypeShift) == PackageImport;
	}

	__forceinline uint32_t ToExport() const
	{
		return uint32_t(TypeAndId);
	}

	__forceinline uint64_t Value() const
	{
		return TypeAndId & IndexMask;
	}

	__forceinline bool operator==(FPackageObjectIndex Other) const
	{
		return TypeAndId == Other.TypeAndId;
	}

	__forceinline bool operator!=(FPackageObjectIndex Other) const
	{
		return TypeAndId != Other.TypeAndId;
	}

	__forceinline uint32_t GetImportedPackageIndex() const
	{
		return static_cast<uint32_t>((TypeAndId & IndexMask) >> 32);
	}

	__forceinline friend uint32_t hash_value(const FPackageObjectIndex& Value)
	{
		return uint32_t(Value.TypeAndId);
	}
};