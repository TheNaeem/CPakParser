#include "Compression.h"

static int32_t FCompression::GetMaximumCompressedSize(std::string& FormatName, int32_t UncompressedSize, ECompressionFlags Flags, int32_t CompressionData)
{
	if (FormatName == "")
	{
		return UncompressedSize;
	}
	else if (FormatName == "Oodle")
	{
		return Oodle::GetMaximumCompressedSize(UncompressedSize);
	}
	else
	{
		return CompressMemoryBound(FormatName, UncompressedSize, Flags, CompressionData);
	}
}

static int32_t FCompression::CompressMemoryBound(std::string& FormatName, int32_t UncompressedSize, ECompressionFlags Flags, int32_t CompressionData) // TODO: this
{
	int32_t CompressionBound = UncompressedSize;

	if (FormatName == "")
	{
		return UncompressedSize;
	}
	else if (FormatName == "Zlib")
	{
	}
	else if (FormatName == "Gzip")
	{
	}
	else if (FormatName == "LZ4")
	{
	}
	else
	{
	}

	return CompressionBound;
}