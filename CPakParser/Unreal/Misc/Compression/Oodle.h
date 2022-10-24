#pragma once

#include <cstdint>

#define OODLELZ_BLOCK_LEN (1<<18) 
#define OODLELZ_BLOCK_MAXIMUM_EXPANSION (2)

typedef intptr_t(*OodleDecompressFunc)(
	const void* compBuf,
	intptr_t compBufSize,
	void* rawBuf,
	intptr_t rawLen,
	int fuzzSafe,
	int checkCRC,
	int verbosity,
	void* decBufBase,
	intptr_t decBufSize,
	void* fpCallback,
	void* callbackUserData,
	void* decoderMemory,
	intptr_t decoderMemorySize,
	uint32_t threadPhase);

namespace Oodle
{
	inline OodleDecompressFunc OodleLZ_Decompress;

	void LoadDLL(const char* DllPath);
	void Decompress(const void* compressedData, intptr_t compressedSize, void* outDecompressedData, intptr_t decompressedSize);
	int64_t GetMaximumCompressedSize(int64_t InUncompressedSize);
};