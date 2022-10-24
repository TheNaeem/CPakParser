#include "BorrowedArchive.h"

#include "FileReader.h"

FBorrowedArchive::FBorrowedArchive(FArchive* InArchive, FPakReaderCollection* InOwner)
	: Archive(InArchive), Owner(InOwner)
{
}

FBorrowedArchive::~FBorrowedArchive()
{
	if (Archive && Owner)
	{
		Owner->ReturnReader(Archive);
		Archive = nullptr;
	}
}

FBorrowedArchive::operator bool() const { return Archive != nullptr; }
bool FBorrowedArchive::operator==(nullptr_t) { return Archive == nullptr; }
bool FBorrowedArchive::operator!=(nullptr_t) { return Archive != nullptr; }
FArchive* FBorrowedArchive::operator->() { return Archive; }
FArchive& FBorrowedArchive::GetArchive() { return *Archive; }

FPakReaderCollection::FPakReaderCollection(std::string InPakFilePath) 
	: PakFilePath(InPakFilePath)
{
}

FPakReaderCollection::~FPakReaderCollection()
{
	for (auto Reader : Readers)
		delete Reader;
}

FBorrowedArchive FPakReaderCollection::BorrowReader()
{
	FArchive* PakReader = nullptr;

	{
		SCOPE_LOCK(CriticalSection);

		if (Readers.size() > 0)
		{
			PakReader = Readers.back();
			Readers.pop_back();
		}
		else
		{
			PakReader = new FFileReader(PakFilePath.c_str());
		}

		++CurrentlyUsedReaders;
	}

	return FBorrowedArchive(PakReader, this);
}

void FPakReaderCollection::ReturnReader(FArchive* SharedReader)
{
	SCOPE_LOCK(CriticalSection);

	--CurrentlyUsedReaders;

	Readers.push_back(SharedReader);
}