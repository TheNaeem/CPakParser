#pragma once

#include <inttypes.h>

#define OODLELZ_BLOCK_LEN   (1<<18) 
#define OODLELZ_BLOCK_MAXIMUM_EXPANSION (2)
#define OODLELZ_BLOCK_MAX_COMPLEN       (OODLELZ_BLOCK_LEN+OODLELZ_BLOCK_MAXIMUM_EXPANSION) 
#define OODLELZ_QUANTUM_LEN     (1<<14) 
#define OODLELZ_QUANTUM_MAXIMUM_EXPANSION   (5)
#define OODLELZ_QUANTUM_MAX_COMPLEN     (OODLELZ_QUANTUM_LEN+OODLELZ_QUANTUM_MAXIMUM_EXPANSION)
#define OODLELZ_SEEKCHUNKLEN_MIN        OODLELZ_BLOCK_LEN
#define OODLELZ_SEEKCHUNKLEN_MAX        (1<<29) 

namespace Oodle
{
	static int64_t GetMaximumCompressedSize(int64_t InUncompressedSize)
	{
		int64_t NumBlocks = (InUncompressedSize + OODLELZ_BLOCK_LEN - 1) / OODLELZ_BLOCK_LEN;
		int64_t MaxCompressedSize = InUncompressedSize + NumBlocks * OODLELZ_BLOCK_MAXIMUM_EXPANSION;

		return MaxCompressedSize;
	}
}