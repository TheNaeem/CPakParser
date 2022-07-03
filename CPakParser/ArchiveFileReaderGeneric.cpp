#include "ArchiveFileReaderGeneric.h"

FArchiveFileReaderGeneric::FArchiveFileReaderGeneric(const char* InFilename) : FileStream(InFilename, std::ios::binary)
{
}

FArchiveFileReaderGeneric::~FArchiveFileReaderGeneric()
{
	FileStream.close();
}

void FArchiveFileReaderGeneric::Seek(int64_t InPos)
{
	FileStream.seekg(InPos, FileStream._Seekbeg);
}

int64_t FArchiveFileReaderGeneric::Tell()
{
	return FileStream.tellg();
}

int64_t FArchiveFileReaderGeneric::TotalSize()
{
	auto Pos = FileStream.tellg();
	FileStream.seekg(0, FileStream._Seekend);

	auto Ret = FileStream.tellg();
	FileStream.seekg(Pos, FileStream._Seekbeg);

	return Ret;
}

bool FArchiveFileReaderGeneric::Close()
{
	FileStream.close();

	return !FileStream.is_open();
}

void FArchiveFileReaderGeneric::Serialize(void* V, uint64_t Length)
{
	FileStream.read(static_cast<char*>(V), Length);
}