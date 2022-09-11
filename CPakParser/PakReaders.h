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

class FPakReader
{
protected:

	std::shared_ptr<FPakFile> Pak;
	FPakEntry Entry;
	int64_t ReadPos;
	FPakDefaultEncryption Encryption;

public:

	FPakReader(std::shared_ptr<FPakFile> PakFile, FPakEntry InEntry, bool bEncrypted) :
		Pak(PakFile),
		Entry(InEntry),
		ReadPos(0),
		Encryption(bEncrypted ? FPakDefaultEncryption() : FPakDefaultEncryption()) 
	{
	}

	virtual void Serialize(int64_t DesiredPosition, void* V, int64_t Length) = 0;

	__forceinline void Read(uint8_t* Destination, int64_t BytesToRead)
	{
		Serialize(ReadPos, Destination, BytesToRead);
		ReadPos += BytesToRead;
	}
};

class FCompressedPakReader : public FPakReader
{
public:

	virtual void Serialize(int64_t DesiredPosition, void* V, int64_t Length) override;
};