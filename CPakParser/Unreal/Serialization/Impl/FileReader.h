#pragma once

#include "../Archives.h"
#include <fstream>

//TODO: memory mapped archive file reader

class FFileReader : public FArchive
{
public:
	FFileReader(const char* InFilename);
	~FFileReader();

	virtual __forceinline void Seek(int64_t InPos) override;
	virtual __forceinline int64_t Tell() override;
	virtual int64_t TotalSize() override;
	bool Close();
	virtual void Serialize(void* V, int64_t Length) override;
	bool IsValid();

protected:

	std::ifstream FileStream;
};