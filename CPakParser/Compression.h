#pragma once

#include <string>
#include "Enums.h"


namespace FCompression
{
	bool VerifyCompressionFlagsValid(int32_t InCompressionFlags);
	int64_t GetMaximumCompressedSize(const std::string& FormatName, int32_t UncompressedSize, ECompressionFlags Flags = COMPRESS_NoFlags, int32_t CompressionData = 0);
	int64_t CompressMemoryBound(const std::string& FormatName, int32_t UncompressedSize, ECompressionFlags Flags, int32_t CompressionData);
	void DecompressMemory(const std::string& FormatName, void* UncompressedBuffer, int32_t UncompressedSize, const void* CompressedBuffer, int32_t CompressedSize);
}