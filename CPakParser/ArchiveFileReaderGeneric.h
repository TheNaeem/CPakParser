#pragma once

#include "Archives.h"
#include <fstream>

//TODO: memory mapped archive file reader

class FArchiveFileReaderGeneric : public FArchive
{
public:
	FArchiveFileReaderGeneric(const char* InFilename);
	~FArchiveFileReaderGeneric();

	virtual __forceinline void Seek(int64_t InPos) override;
	virtual __forceinline int64_t Tell() override;
	virtual int64_t TotalSize() override;
	__forceinline bool Close();
	virtual void Serialize(void* V, uint64_t Length) override;

protected:

	std::ifstream FileStream;
};