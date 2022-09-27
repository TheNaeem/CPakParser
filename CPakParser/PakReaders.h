#pragma once

#include "Archives.h"
#include "PakFiles.h"

class FPakDefaultEncryption
{
public:

	enum
	{
		Alignment = 1,
	};

	virtual __forceinline int64_t AlignReadRequest(int64_t Size) 
	{
		return Size;
	}

	virtual __forceinline void DecryptBlock(void* Data, int64_t Size, const FGuid& EncryptionKeyGuid) 
	{
	}
};

