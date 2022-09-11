#include "PakReaders.h"
#include "Compression.h"

class FAsyncPakDecompressor
{

};

void FCompressedPakReader::Serialize(int64_t DesiredPosition, void* V, int64_t Length)
{
	int32_t CompressionBlockSize = Entry.CompressionBlockSize;
	uint32_t CompressionBlockIndex = DesiredPosition / CompressionBlockSize;
	uint8_t* WorkingBuffers[2];
	int64_t DirectCopyStart = DesiredPosition % CompressionBlockSize;

	auto CompressionMethod = Pak->GetInfo().GetCompressionMethod(Entry.CompressionMethodIndex);

	auto WorkingBufferRequiredSize = Encryption.AlignReadRequest(FCompression::GetMaximumCompressedSize(CompressionMethod, CompressionBlockSize));
	const bool bExistingScratchBufferValid = ScratchSpace->TempBufferSize >= CompressionBlockSize;
}