#pragma once

#include "Core/Defines.h"
#include "Misc/Encryption/EncryptionKeyManager.h"

import CPakParser.Paks.PakEntry;
import CPakParser.Serialization.FArchive;
import CPakParser.Paks.PakFile;

export class FPakNoEncryption
{
public:
	enum
	{
		Alignment = 1,
	};

	static __forceinline int64_t AlignReadRequest(int64_t Size)
	{
		return Size;
	}

	static __forceinline void DecryptBlock(void* Data, int64_t Size, const FGuid& EncryptionKeyGuid, FEncryptionKeyManager& KeyManager)
	{
	}
};

export class FPakSimpleEncryption
{
public:

	enum
	{
		Alignment = FAESKey::AESBlockSize,
	};

	static __forceinline int64_t AlignReadRequest(int64_t Size)
	{
		return Align(Size, Alignment);
	}

	static __forceinline void DecryptBlock(uint8_t* Data, int64_t Size, const FGuid& EncryptionKeyGuid, FEncryptionKeyManager& KeyManager)
	{
		FAESKey Key;
		if (KeyManager.GetKey(EncryptionKeyGuid, Key) && Key.IsValid())
		{
			Key.DecryptData(Data, Size);
		}
	}
};

template <typename Encryption = FPakNoEncryption>
class FPakReader : public FArchive
{
	FEncryptionKeyManager& KeyManager;
	int64_t OffsetToFile;
	int64_t ReadPos;
	TSharedPtr<FPakFile> Pak;
	FPakEntry Entry;
	bool bCompressed;

public:

	FPakReader(TSharedPtr<FPakFile> PakFile, FPakEntry InEntry, FEncryptionKeyManager& EncryptionKeyManager);

	virtual void Seek(int64_t InPos) override;
	virtual int64_t Tell() override;
	virtual int64_t TotalSize() override;
	virtual void Serialize(void* V, int64_t Length) override;

private:

	void SerializeInternal(void* V, int64_t Length);
	void SerializeInternalCompressed(void* V, int64_t Length);
};