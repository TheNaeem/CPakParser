#include "PakInfo.h"

#include "Serialization/Archives.h"
#include "Misc/Hashing/ShaHash.h"
#include "Logger.h"

int64_t FPakInfo::GetSerializedSize(int32_t InVersion) const
{
	int64_t Size = sizeof(Magic) + sizeof(Version) + sizeof(IndexOffset) + sizeof(IndexSize) + sizeof(FSHAHash) + sizeof(bEncryptedIndex);
	if (InVersion >= PakFile_Version_EncryptionKeyGuid) Size += sizeof(EncryptionKeyGuid);
	if (InVersion >= PakFile_Version_FNameBasedCompressionMethod) Size += CompressionMethodNameLen * MaxNumCompressionMethods;
	if (InVersion >= PakFile_Version_FrozenIndex && InVersion < PakFile_Version_PathHashIndex) Size += sizeof(bool);

	return Size;
}

void FPakInfo::Serialize(FArchive& Ar, int32_t InVersion)
{
	if (Ar.TotalSize() < (Ar.Tell() + GetSerializedSize(InVersion)))
	{
		Magic = 0;
		return;
	}

	if (InVersion >= PakFile_Version_EncryptionKeyGuid)
	{
		Ar << EncryptionKeyGuid;
	}

	Ar << bEncryptedIndex;
	Ar << Magic;

	if (Magic != PakFile_Magic)
		return;

	Ar << Version;
	Ar << IndexOffset;
	Ar << IndexSize;
	Ar.Seek(Ar.Tell() + sizeof(FSHAHash)); // IndexHash

	if (Version < PakFile_Version_IndexEncryption)
	{
		bEncryptedIndex = false;
	}

	if (Version < PakFile_Version_EncryptionKeyGuid)
	{
		EncryptionKeyGuid.Invalidate();
	}

	if (Version >= PakFile_Version_FrozenIndex && Version < PakFile_Version_PathHashIndex)
	{
		bool bIndexIsFrozen = false;
		Ar << bIndexIsFrozen;

		if (bIndexIsFrozen)
			Log<Error>("Pak frozen with unsupported version");
	}

	if (Version < PakFile_Version_FNameBasedCompressionMethod)
	{
		CompressionMethods.push_back("Zlib");
		CompressionMethods.push_back("Gzip");
		CompressionMethods.push_back("Oodle");

		return;
	}

	int BufSize = CompressionMethodNameLen * MaxNumCompressionMethods;
	auto Methods = std::make_unique<char[]>(BufSize);

	Ar.Serialize(Methods.get(), BufSize);

	for (size_t i = 0; i < MaxNumCompressionMethods; i++)
	{
		const char* MethodString = &Methods[i * CompressionMethodNameLen];

		if (MethodString[0] != 0)
		{
			CompressionMethods.push_back(MethodString);
		}
	}
}