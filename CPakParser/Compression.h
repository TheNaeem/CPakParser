#pragma once

#include "Oodle.h"
#include <string>
#include "Enums.h"


namespace FCompression
{
	static int32_t GetMaximumCompressedSize(std::string& FormatName, int32_t UncompressedSize, ECompressionFlags Flags = COMPRESS_NoFlags, int32_t CompressionData = 0);
	static int32_t CompressMemoryBound(std::string& FormatName, int32_t UncompressedSize, ECompressionFlags Flags, int32_t CompressionData);
}