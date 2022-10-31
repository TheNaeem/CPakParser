#pragma once

#include <cstdint>

class FPackageIndex
{
	/**
	 * Values greater than zero indicate that this is an index into the ExportMap.  The
	 * actual array index will be (FPackageIndex - 1).
	 *
	 * Values less than zero indicate that this is an index into the ImportMap. The actual
	 * array index will be (-FPackageIndex - 1)
	 */
	int32_t Index;

	/** Internal constructor, sets the index directly **/
	__forceinline explicit FPackageIndex(int32_t InIndex)
		: Index(InIndex)
	{
	}

public:
	/** Constructor, sets the value to null **/
	__forceinline FPackageIndex()
		: Index(0)
	{

	}
	/** return true if this is an index into the import map **/
	__forceinline bool IsImport() const
	{
		return Index < 0;
	}
	/** return true if this is an index into the export map **/
	__forceinline bool IsExport() const
	{
		return Index > 0;
	}
	/** return true if this null (i.e. neither an import nor an export) **/
	__forceinline bool IsNull() const
	{
		return Index == 0;
	}
	/** Check that this is an import and return the index into the import map **/
	__forceinline uint32_t ToImport() const
	{
		return -Index - 1;
	}
	/** Check that this is an export and return the index into the export map **/
	__forceinline uint32_t ToExport() const
	{
		return Index - 1;
	}
	/** Return the raw value, for debugging purposes**/
	__forceinline uint32_t ForDebugging() const
	{
		return Index;
	}

	/** Compare package indecies for equality **/
	__forceinline bool operator==(const FPackageIndex& Other) const
	{
		return Index == Other.Index;
	}
	/** Compare package indecies for inequality **/
	__forceinline bool operator!=(const FPackageIndex& Other) const
	{
		return Index != Other.Index;
	}

	/** Compare package indecies **/
	__forceinline bool operator<(const FPackageIndex& Other) const
	{
		return Index < Other.Index;
	}
	__forceinline bool operator>(const FPackageIndex& Other) const
	{
		return Index > Other.Index;
	}
	__forceinline bool operator<=(const FPackageIndex& Other) const
	{
		return Index <= Other.Index;
	}
	__forceinline bool operator>=(const FPackageIndex& Other) const
	{
		return Index >= Other.Index;
	}
	/**
	 * Serializes a package index value from or into an archive.
	 *
	 * @param Ar - The archive to serialize from or to.
	 * @param Value - The value to serialize.
	 */
	friend class FArchive& operator<<(FArchive& Ar, FPackageIndex& Value);

	__forceinline friend uint32_t hash_value(const FPackageIndex& In)
	{
		return uint32_t(In.Index);
	}
};