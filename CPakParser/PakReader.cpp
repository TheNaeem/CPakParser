#include "PakReader.h"

#include "Compression.h"
#include <future>

/*
* I spent quite a bit of time thinking about how to approach handling decompression buffers in an efficient way.
* I concluded that what UE already had implemented is pretty well thought out and already gets the job done.
* So, I'll just do exactly what they do for now. This should be revisited in the future though.
*/

struct FCompressionScratchBuffers
{
	FCompressionScratchBuffers()
		: TempBufferSize(0)
		, ScratchBufferSize(0)
		, LastPakEntryOffset(-1)
		, LastDecompressedBlock(0xFFFFFFFF)
		, Next(nullptr)
	{}

	int64_t	TempBufferSize;
	TUniquePtr<uint8_t[]> TempBuffer;
	int64_t	ScratchBufferSize;
	TUniquePtr<uint8_t[]> ScratchBuffer;

	int64_t LastPakEntryOffset;
	int64_t LastDecompressedBlock;

	FCompressionScratchBuffers* Next;

	void EnsureBufferSpace(int64_t CompressionBlockSize, int64_t ScrachSize)
	{
		if (TempBufferSize < CompressionBlockSize)
		{
			TempBufferSize = CompressionBlockSize;
			TempBuffer = std::make_unique<uint8_t[]>(TempBufferSize);
		}

		if (ScratchBufferSize < ScrachSize)
		{
			ScratchBufferSize = ScrachSize;
			ScratchBuffer = std::make_unique<uint8_t[]>(ScratchBufferSize);
		}
	}
};

class FCompressionScratchBuffersStack
{
public:

	FCompressionScratchBuffersStack()
		: bFirstInUse(false)
		, RecursionList(nullptr)
	{}

private:

	static __forceinline FCompressionScratchBuffersStack& Get()
	{
		static FCompressionScratchBuffersStack Inst;
		return Inst;
	}

	static FCompressionScratchBuffers* Acquire()
	{
		if (!Get().bFirstInUse)
		{
			Get().bFirstInUse = true;
			return &Get().First;
		}

		FCompressionScratchBuffers* Top = new FCompressionScratchBuffers;
		Top->Next = Get().RecursionList;
		Get().RecursionList = Top;

		return Top;
	}

	static void Release(FCompressionScratchBuffers* Top)
	{
		if (!Get().RecursionList)
		{
			Get().bFirstInUse = false;
			return;
		}

		Get().RecursionList = Top->Next;
		delete Top;
	}

	bool bFirstInUse;
	FCompressionScratchBuffers First;
	FCompressionScratchBuffers* RecursionList;

	friend class FScopedCompressionScratchBuffers;
};

class FScopedCompressionScratchBuffers
{
public:
	FScopedCompressionScratchBuffers()
		: Inner(FCompressionScratchBuffersStack::Acquire())
	{}

	~FScopedCompressionScratchBuffers()
	{
		FCompressionScratchBuffersStack::Release(Inner);
	}

	FCompressionScratchBuffers* operator->() const
	{
		return Inner;
	}

private:
	FCompressionScratchBuffers* Inner;
};

template <typename T>
int64_t FPakReader<T>::Tell()
{
	return ReadPos;
}

template <typename T>
int64_t FPakReader<T>::TotalSize()
{
	return bCompressed ? Entry.UncompressedSize : Entry.Size;
}

template <typename T>
void FPakReader<T>::Seek(int64_t InPos)
{
	ReadPos = InPos;
}

template <typename Encryption>
void FPakReader<Encryption>::SerializeInternal(void* V, int64_t Length)
{
	FGuid EncryptionKeyGuid = Pak->GetInfo().EncryptionKeyGuid;
	const constexpr int64_t Alignment = (int64_t)Encryption::Alignment;
	const constexpr int64_t AlignmentMask = ~(Alignment - 1);
	uint8_t TempBuffer[Alignment];

	auto Reader = Pak->GetSharedReader();

	if (Encryption::AlignReadRequest(ReadPos) != ReadPos)
	{
		int64_t Start = ReadPos & AlignmentMask;
		int64_t Offset = ReadPos - Start;
		int64_t CopySize = std::min(Alignment - Offset, Length);

		Reader->Seek(OffsetToFile + Start);
		Reader->Serialize(TempBuffer, Alignment);

		Encryption::DecryptBlock(TempBuffer, Alignment, EncryptionKeyGuid, KeyManager);
		memcpy(V, TempBuffer + Offset, CopySize);

		V = (void*)((uint8_t*)V + CopySize);
		ReadPos += CopySize;
		Length -= CopySize;
	}
	else
	{
		Reader->Seek(OffsetToFile + ReadPos);
	}

	int64_t CopySize = Length & AlignmentMask;
	Reader->Serialize(V, CopySize);
	Encryption::DecryptBlock(static_cast<uint8_t*>(V), CopySize, EncryptionKeyGuid, KeyManager);
	Length -= CopySize;
	V = (void*)((uint8_t*)V + CopySize);

	if (Length > 0)
	{
		Reader->Serialize(TempBuffer, Alignment);
		Encryption::DecryptBlock(TempBuffer, Alignment, EncryptionKeyGuid, KeyManager);
		memcpy(V, TempBuffer, Length);
	}
}

// TODO: this. its uneccesary if we're just gonna decompress the whole thing at once and serialize from memory. 
template <typename Encryption>
void FPakReader<Encryption>::SerializeInternalCompressed(void* V, int64_t Length)
{
	static auto DoDecompression = [&KeyManager = KeyManager](
		const std::string CompressionFormat,
		uint8_t* CompressedBuffer,
		int32_t CompressedSize,
		uint8_t* DecompressedBuffer,
		int32_t DecompressedSize,
		FGuid KeyGuid,
		int64_t	CopyOffset,
		int64_t CopyLength,
		void* CopyOut)
	{
		int64_t EncryptedSize = Encryption::AlignReadRequest(CompressedSize);

		Encryption::DecryptBlock(CompressedBuffer, EncryptedSize, KeyGuid, KeyManager);
		FCompression::DecompressMemory(CompressionFormat, DecompressedBuffer, DecompressedSize, CompressedBuffer, CompressedSize);

		if (CopyOut)
			memcpy(CopyOut, DecompressedBuffer + CopyOffset, CopyLength);
	};

	const auto CompressionBlockSize = Entry.CompressionBlockSize;
	auto CompressionBlockIndex = ReadPos / CompressionBlockSize;
	int64_t DirectCopyStart = ReadPos % Entry.CompressionBlockSize;
	FScopedCompressionScratchBuffers ScratchSpace;
	uint8_t* WorkingBuffers[2];
	std::future<void> DecompressionTask;

	auto CompressionMethod = Pak->GetInfo().GetCompressionMethod(Entry.CompressionMethodIndex);

	auto WorkingBufferRequiredSize = FCompression::GetMaximumCompressedSize(CompressionMethod, CompressionBlockSize);
	WorkingBufferRequiredSize = Encryption::AlignReadRequest(WorkingBufferRequiredSize);
	ScratchSpace->EnsureBufferSpace(CompressionBlockSize, WorkingBufferRequiredSize * 2);

	auto PakReader = Pak->GetSharedReader();
	bool bIsDecompressing = false;
	const bool bExistingScratchBufferValid = ScratchSpace->TempBufferSize >= CompressionBlockSize;

	WorkingBuffers[0] = ScratchSpace->ScratchBuffer.get();
	WorkingBuffers[1] = ScratchSpace->ScratchBuffer.get() + WorkingBufferRequiredSize;

	while (Length > 0)
	{
		const FPakCompressedBlock& Block = Entry.CompressionBlocks[CompressionBlockIndex];

		int64_t Pos = CompressionBlockIndex * CompressionBlockSize;
		int64_t CompressedBlockSize = Block.CompressedEnd - Block.CompressedStart;
		int64_t UncompressedBlockSize = std::min(Entry.UncompressedSize - Pos, (int64_t)Entry.CompressionBlockSize);

		int64_t ReadSize = Encryption::AlignReadRequest(CompressedBlockSize);
		int64_t WriteSize = std::min(UncompressedBlockSize - DirectCopyStart, Length);

		static auto NextIteration = [&]()
		{
			V = (void*)((uint8_t*)V + WriteSize);
			Length -= WriteSize;
			DirectCopyStart = 0;
			++CompressionBlockIndex;
		};

		const bool bCurrentScratchTempBufferValid =
			bExistingScratchBufferValid && !bIsDecompressing
			&& (ScratchSpace->LastPakEntryOffset == Entry.Offset)
			&& (ScratchSpace->LastDecompressedBlock == CompressionBlockIndex)
			&& !(DirectCopyStart == 0 && Length >= CompressionBlockSize);

		if (bCurrentScratchTempBufferValid)
		{
			memcpy(V, ScratchSpace->TempBuffer.get() + DirectCopyStart, WriteSize);
			NextIteration();
			continue;
		}

		PakReader->Seek(Block.CompressedStart + (Pak->GetInfo().HasRelativeCompressedChunkOffsets() ? Entry.Offset : 0));
		PakReader->Serialize(WorkingBuffers[CompressionBlockIndex & 1], ReadSize);

		if (bIsDecompressing)
		{
			DecompressionTask.wait();
			bIsDecompressing = false;
		}

		auto Guid = Pak->GetInfo().EncryptionKeyGuid;

		bool bCopy = DirectCopyStart != 0 || Length < CompressionBlockSize;

		ScratchSpace->LastDecompressedBlock = bCopy ? CompressionBlockIndex : 0xFFFFFFFF;
		ScratchSpace->LastPakEntryOffset = bCopy ? Entry.Offset : -1;

		DecompressionTask = std::async(std::launch::async, DoDecompression,
			CompressionMethod,
			WorkingBuffers[CompressionBlockIndex & 1],
			CompressedBlockSize,
			bCopy ? ScratchSpace->TempBuffer.get() : static_cast<uint8_t*>(V),
			UncompressedBlockSize,
			Guid,
			DirectCopyStart,
			WriteSize,
			bCopy ? V : nullptr);

		if (Length == WriteSize)
			DecompressionTask.wait();

		bIsDecompressing = true;
		NextIteration();
	}

	if (bIsDecompressing)
		DecompressionTask.wait();
}

template <typename T>
void FPakReader<T>::Serialize(void* V, int64_t Length)
{
	if (TotalSize() < (ReadPos + Length))
	{
		return;
	}

	auto PosCopy = ReadPos;

	if (bCompressed)
	{
		SerializeInternalCompressed(V, Length);
		ReadPos = PosCopy + Length;
		return;
	}
	
	SerializeInternal(V, Length);
	ReadPos = PosCopy + Length;
}

template class FPakReader<FPakNoEncryption>;
template class FPakReader<FPakSimpleEncryption>;