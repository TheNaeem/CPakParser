#include "Compression.h"

#include "Oodle.h"

bool FCompression::VerifyCompressionFlagsValid(int32_t InCompressionFlags)
{
	const int32_t CompressionFlagsMask = COMPRESS_DeprecatedFormatFlagsMask | COMPRESS_OptionsFlagsMask | COMPRESS_ForPurposeMask;
	if (InCompressionFlags & (~CompressionFlagsMask))
	{
		return false;
	}

	return true;
}

int64_t FCompression::GetMaximumCompressedSize(const std::string& FormatName, int32_t UncompressedSize, ECompressionFlags Flags, int32_t CompressionData)
{
	if (FormatName == "Oodle")
		return Oodle::GetMaximumCompressedSize(UncompressedSize);

	return CompressMemoryBound(FormatName, UncompressedSize, Flags, CompressionData);
}

int64_t FCompression::CompressMemoryBound(const std::string& FormatName, int32_t UncompressedSize, ECompressionFlags Flags, int32_t CompressionData)
{
	int32_t CompressionBound = UncompressedSize;

	if (FormatName.empty())
	{
		return UncompressedSize;
	}
	else if (FormatName == "Zlib")
	{
		// TODO
	}
	else if (FormatName == "Gzip")
	{
		// TODO
	}
	else if (FormatName == "LZ4")
	{
		// TODO
	}

	return CompressionBound;
	
}

void FCompression::DecompressMemory(const std::string& FormatName, void* UncompressedBuffer, int32_t UncompressedSize, const void* CompressedBuffer, int32_t CompressedSize)
{
	if (FormatName == "Zlib")
	{
		// TODO
	}
	else if (FormatName == "Gzip")
	{
		// TODO
	}
	else if (FormatName == "LZ4")
	{
		// TODO
	}
	else if (FormatName == "Oodle")
	{
		Oodle::Decompress(CompressedBuffer, CompressedSize, UncompressedBuffer, UncompressedSize);
	}
}