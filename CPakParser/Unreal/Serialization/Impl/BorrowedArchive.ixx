export module BorrowedArchive;

import DiskFile;
import <vector>;
import "Misc/Multithreading/Lock.h";

export import FArchiveBase;

export class FBorrowedArchive final // TODO: how to implement this across Tocs and Paks
{
	friend class FPakReaderCollection;

	FArchive* Archive = nullptr;
	class FPakReaderCollection* Owner = nullptr;

	FBorrowedArchive(FArchive* InArchive, class FPakReaderCollection* InOwner);

public:

	~FBorrowedArchive();

	FBorrowedArchive(const FBorrowedArchive& Other) = delete;
	FBorrowedArchive& operator=(const FBorrowedArchive& Other) = delete;

	explicit operator bool() const;
	bool operator==(nullptr_t);
	bool operator!=(nullptr_t);
	FArchive* operator->();
	FArchive& GetArchive();
};

export class FPakReaderCollection
{
public:

	FPakReaderCollection(std::string InPakFilePath);
	~FPakReaderCollection();

private:

	int32_t CurrentlyUsedReaders = 0;
	std::vector<FArchive*> Readers;
	std::mutex CriticalSection;
	std::string PakFilePath;

public:

	FBorrowedArchive BorrowReader();
	void ReturnReader(FArchive* SharedReader);
};