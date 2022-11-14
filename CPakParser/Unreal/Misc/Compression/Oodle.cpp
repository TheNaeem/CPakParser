#include <filesystem>
#include <Windows.h>

import Oodle;
import Logging;

#define OODLELZ_BLOCK_LEN (1<<18) 
#define OODLELZ_BLOCK_MAXIMUM_EXPANSION (2)

void Oodle::LoadDLL(const char* DllPath)
{
	if (!std::filesystem::exists(DllPath))
	{
		LogError("Provided Oodle path does not exist!");
		return;
	}

	auto OodleHandle = LoadLibraryA(DllPath);

	OodleLZ_Decompress = (OodleDecompressFunc)GetProcAddress(LoadLibraryA(DllPath), "OodleLZ_Decompress");
}

void Oodle::Decompress(const void* compressedData, intptr_t compressedSize, void* outDecompressedData, intptr_t decompressedSize)
{
	if (!OodleLZ_Decompress)
	{
		throw std::exception("Oodle decompress is called despite the DLL not being loaded!");
	}

	OodleLZ_Decompress(
		compressedData,
		compressedSize,
		outDecompressedData,
		decompressedSize,
		1, 0, 0, NULL, 0, NULL, NULL, NULL, 0, 3);
}

int64_t Oodle::GetMaximumCompressedSize(int64_t InUncompressedSize)
{
	int64_t NumBlocks = (InUncompressedSize + OODLELZ_BLOCK_LEN - 1) / OODLELZ_BLOCK_LEN;
	int64_t MaxCompressedSize = InUncompressedSize + NumBlocks * OODLELZ_BLOCK_MAXIMUM_EXPANSION;
	return MaxCompressedSize;
}