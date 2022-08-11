#include "FileReader.h"

FFileReader::FFileReader(const char* InFilename) : FileStream(InFilename, std::ios::binary)
{
}

FFileReader::~FFileReader()
{
	FileStream.close();
}

void FFileReader::Seek(int64_t InPos)
{
	FileStream.seekg(InPos, FileStream._Seekbeg);
}

int64_t FFileReader::Tell()
{
	return FileStream.tellg();
}

int64_t FFileReader::TotalSize()
{
	auto Pos = FileStream.tellg();
	FileStream.seekg(0, FileStream._Seekend);

	auto Ret = FileStream.tellg();
	FileStream.seekg(Pos, FileStream._Seekbeg);

	return Ret;
}

bool FFileReader::Close()
{
	FileStream.close();

	return !FileStream.is_open();
}

void FFileReader::Serialize(void* V, int64_t Length)
{
	FileStream.read(static_cast<char*>(V), Length);
}

bool FFileReader::IsValid() 
{
	return !!FileStream;
}