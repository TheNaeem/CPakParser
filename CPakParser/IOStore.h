#pragma once

#include "Hashing.h"
#include "AES.h"
#include <span>
#include <shared_mutex>

typedef std::unique_lock<std::shared_mutex> WriteLock;

class FArchive;

class FIoContainerId
{
public:
	inline FIoContainerId() = default;
	inline FIoContainerId(const FIoContainerId& Other) = default;
	inline FIoContainerId(FIoContainerId&& Other) = default;
	inline FIoContainerId& operator=(const FIoContainerId& Other) = default;

	static FIoContainerId FromName(const FName& Name);

	uint64_t Value() const
	{
		return Id;
	}

	inline bool IsValid() const
	{
		return Id != InvalidId;
	}

	inline bool operator<(FIoContainerId Other) const
	{
		return Id < Other.Id;
	}

	inline bool operator==(FIoContainerId Other) const
	{
		return Id == Other.Id;
	}

	inline bool operator!=(FIoContainerId Other) const
	{
		return Id != Other.Id;
	}

	inline friend uint32_t GetTypeHash(const FIoContainerId& In)
	{
		return uint32_t(In.Id);
	}

	friend FArchive& operator<<(FArchive& Ar, FIoContainerId& ContainerId);

private:
	inline explicit FIoContainerId(const uint64_t InId)
		: Id(InId) { }

	static constexpr uint64_t InvalidId = uint64_t(-1);

	uint64_t Id;
};

struct FIoContainerHeaderPackageRedirect
{
	FPackageId SourcePackageId;
	FPackageId TargetPackageId;
	FMappedName SourcePackageName;

	friend FArchive& operator<<(FArchive& Ar, FIoContainerHeaderPackageRedirect& PackageRedirect);
};

struct FIoContainerHeaderLocalizedPackage
{
	FPackageId SourcePackageId;
	FMappedName SourcePackageName;

	friend FArchive& operator<<(FArchive& Ar, FIoContainerHeaderLocalizedPackage& LocalizedPackage);
};

struct FIoContainerHeader
{
	enum
	{
		Signature = 0x496f436e
	};

	enum class Version : uint32_t
	{
		Initial = 0,
		LocalizedPackages = 1,
		OptionalSegmentPackages = 2,

		LatestPlusOne,
		Latest = LatestPlusOne - 1
	};

	FIoContainerId ContainerId;
	std::vector<FPackageId> PackageIds;
	std::vector<uint8_t> StoreEntries;
	std::vector<FPackageId> OptionalSegmentPackageIds;
	std::vector<uint8_t> OptionalSegmentStoreEntries;
	std::vector<std::string> RedirectsNameMap;
	std::vector<FIoContainerHeaderLocalizedPackage> LocalizedPackages;
	std::vector<FIoContainerHeaderPackageRedirect> PackageRedirects;

	friend FArchive& operator<<(FArchive& Ar, FIoContainerHeader& ContainerHeader);
};

class FIoChunkId
{
public:
	static const FIoChunkId InvalidChunkId;

	friend std::string LexToString(const FIoChunkId& Id);

	inline bool operator ==(const FIoChunkId& Rhs) const
	{
		return 0 == memcmp(Id, Rhs.Id, sizeof Id);
	}

	inline bool operator !=(const FIoChunkId& Rhs) const
	{
		return !(*this == Rhs);
	}

	void Set(const void* InIdPtr, size_t InSize)
	{
		memcpy(Id, InIdPtr, sizeof Id);
	}

	inline bool IsValid() const
	{
		return *this != InvalidChunkId;
	}

	inline const uint8_t* GetData() const { return Id; }
	inline uint32_t	GetSize() const { return sizeof Id; }

	EIoChunkType GetChunkType() const
	{
		return static_cast<EIoChunkType>(Id[11]);
	}

	size_t operator()(FIoChunkId const& in) const noexcept
	{
		uint32_t Hash = 5381;

		for (int i = 0; i < sizeof Id; ++i)
		{
			Hash = Hash * 33 + in.Id[i];
		}

		return Hash;
	}

private:
	static inline FIoChunkId CreateEmptyId()
	{
		FIoChunkId ChunkId;
		uint8_t Data[12] = { 0 };
		ChunkId.Set(Data, sizeof Data);

		return ChunkId;
	}

	uint8_t Id[12];
};

struct FIoOffsetAndLength
{
public:
	inline uint64_t GetOffset() const
	{
		return OffsetAndLength[4]
			| (uint64_t(OffsetAndLength[3]) << 8)
			| (uint64_t(OffsetAndLength[2]) << 16)
			| (uint64_t(OffsetAndLength[1]) << 24)
			| (uint64_t(OffsetAndLength[0]) << 32);
	}

	inline uint64_t GetLength() const
	{
		return OffsetAndLength[9]
			| (uint64_t(OffsetAndLength[8]) << 8)
			| (uint64_t(OffsetAndLength[7]) << 16)
			| (uint64_t(OffsetAndLength[6]) << 24)
			| (uint64_t(OffsetAndLength[5]) << 32);
	}

	inline void SetOffset(uint64_t Offset)
	{
		OffsetAndLength[0] = uint8_t(Offset >> 32);
		OffsetAndLength[1] = uint8_t(Offset >> 24);
		OffsetAndLength[2] = uint8_t(Offset >> 16);
		OffsetAndLength[3] = uint8_t(Offset >> 8);
		OffsetAndLength[4] = uint8_t(Offset >> 0);
	}

	inline void SetLength(uint64_t Length)
	{
		OffsetAndLength[5] = uint8_t(Length >> 32);
		OffsetAndLength[6] = uint8_t(Length >> 24);
		OffsetAndLength[7] = uint8_t(Length >> 16);
		OffsetAndLength[8] = uint8_t(Length >> 8);
		OffsetAndLength[9] = uint8_t(Length >> 0);
	}

private:
	uint8_t OffsetAndLength[5 + 5];
};

struct FIoStoreTocCompressedBlockEntry
{
	static constexpr uint32_t OffsetBits = 40;
	static constexpr uint64_t OffsetMask = (1ull << OffsetBits) - 1ull;
	static constexpr uint32_t SizeBits = 24;
	static constexpr uint32_t SizeMask = (1 << SizeBits) - 1;
	static constexpr uint32_t SizeShift = 8;

	inline uint64_t GetOffset() const
	{
		const uint64_t* Offset = reinterpret_cast<const uint64_t*>(Data);
		return *Offset & OffsetMask;
	}

	inline void SetOffset(uint64_t InOffset)
	{
		uint64_t* Offset = reinterpret_cast<uint64_t*>(Data);
		*Offset = InOffset & OffsetMask;
	}

	inline uint32_t GetCompressedSize() const
	{
		const uint32_t* Size = reinterpret_cast<const uint32_t*>(Data) + 1;
		return (*Size >> SizeShift) & SizeMask;
	}

	inline void SetCompressedSize(uint32_t InSize)
	{
		uint32_t* Size = reinterpret_cast<uint32_t*>(Data) + 1;
		*Size |= (uint32_t(InSize) << SizeShift);
	}

	inline uint32_t GetUncompressedSize() const
	{
		const uint32_t* UncompressedSize = reinterpret_cast<const uint32_t*>(Data) + 2;
		return *UncompressedSize & SizeMask;
	}

	inline void SetUncompressedSize(uint32_t InSize)
	{
		uint32_t* UncompressedSize = reinterpret_cast<uint32_t*>(Data) + 2;
		*UncompressedSize = InSize & SizeMask;
	}

	inline uint8_t GetCompressionMethodIndex() const
	{
		const uint32_t* Index = reinterpret_cast<const uint32_t*>(Data) + 2;
		return static_cast<uint8_t>(*Index >> SizeBits);
	}

	inline void SetCompressionMethodIndex(uint8_t InIndex)
	{
		uint32_t* Index = reinterpret_cast<uint32_t*>(Data) + 2;
		*Index |= uint32_t(InIndex) << SizeBits;
	}

private:
	/* 5 bytes offset, 3 bytes for size / uncompressed size and 1 byte for compresseion method. */
	uint8_t Data[5 + 3 + 3 + 1];
};

struct FFileIoStoreContainerFilePartition
{
	FFileIoStoreContainerFilePartition() = default;
	FFileIoStoreContainerFilePartition(FFileIoStoreContainerFilePartition&&) = default;
	FFileIoStoreContainerFilePartition(const FFileIoStoreContainerFilePartition&) = delete;

	FFileIoStoreContainerFilePartition& operator=(FFileIoStoreContainerFilePartition&&) = default;
	FFileIoStoreContainerFilePartition& operator=(const FFileIoStoreContainerFilePartition&) = delete;

	void* FileHandle = 0;
	uint64_t FileSize = 0;
	uint32_t ContainerFileIndex = 0;
	std::string FilePath;
	std::unique_ptr<IMappedFileHandle> MappedFileHandle;
};

struct FFileIoStoreContainerFile
{
	FFileIoStoreContainerFile() = default;
	FFileIoStoreContainerFile(FFileIoStoreContainerFile&&) = default;
	FFileIoStoreContainerFile(const FFileIoStoreContainerFile&) = delete;

	FFileIoStoreContainerFile& operator=(FFileIoStoreContainerFile&&) = default;
	FFileIoStoreContainerFile& operator=(const FFileIoStoreContainerFile&) = delete;

	uint64_t PartitionSize = 0;
	uint64_t CompressionBlockSize = 0;
	std::vector<std::string> CompressionMethods;
	std::vector<FIoStoreTocCompressedBlockEntry> CompressionBlocks;
	std::string FilePath;
	FGuid EncryptionKeyGuid;
	FAESKey EncryptionKey;
	EIoContainerFlags ContainerFlags;
	std::vector<class FSHAHash> BlockSignatureHashes;
	std::vector<FFileIoStoreContainerFilePartition> Partitions;
	uint32_t ContainerInstanceId = 0;

	void GetPartitionFileHandleAndOffset(uint64_t TocOffset, void*& OutFileHandle, uint64_t& OutOffset) const
	{
		int32_t PartitionIndex = int32_t(TocOffset / PartitionSize);
		const FFileIoStoreContainerFilePartition& Partition = Partitions[PartitionIndex];
		OutFileHandle = Partition.FileHandle;
		OutOffset = TocOffset % PartitionSize;
	}
};

struct FIoStoreTocHeader
{
	static constexpr char TocMagicImg[] = "-==--==--==--==-";

	uint8_t TocMagic[16];
	uint8_t	Version;
	uint8_t	Reserved0 = 0;
	uint16_t Reserved1 = 0;
	uint32_t TocHeaderSize;
	uint32_t TocEntryCount;
	uint32_t TocCompressedBlockEntryCount;
	uint32_t TocCompressedBlockEntrySize;
	uint32_t CompressionMethodNameCount;
	uint32_t CompressionMethodNameLength;
	uint32_t CompressionBlockSize;
	uint32_t DirectoryIndexSize;
	uint32_t PartitionCount = 0;
	FIoContainerId ContainerId;
	FGuid EncryptionKeyGuid;
	EIoContainerFlags ContainerFlags;
	uint8_t	Reserved3 = 0;
	uint16_t Reserved4 = 0;
	uint32_t TocChunkPerfectHashSeedsCount = 0;
	uint64_t PartitionSize = 0;
	uint32_t TocChunksWithoutPerfectHashCount = 0;
	uint32_t Reserved7 = 0;
	uint64_t Reserved8[5] = { 0 };

	void MakeMagic()
	{
		memcpy(TocMagic, TocMagicImg, sizeof TocMagic);
	}

	bool CheckMagic() const
	{
		return memcpy((void*)TocMagic, TocMagicImg, sizeof TocMagic);
	}
};

struct FIoStoreTocEntryMeta
{
	FIoChunkHash ChunkHash;
	FIoStoreTocEntryMetaFlags Flags;
};

struct FIoStoreWriterSettings
{
	FName CompressionMethod = NAME_None;
	uint64_t CompressionBlockSize = 64 << 10;
	uint64_t CompressionBlockAlignment = 0;
	int32_t CompressionMinBytesSaved = 0;
	int32_t CompressionMinPercentSaved = 0;
	int32_t CompressionMinSizeToConsiderDDC = 0;
	uint64_t MemoryMappingAlignment = 0;
	uint64_t MaxPartitionSize = 0;
	bool bEnableCsvOutput = false;
	bool bEnableFileRegions = false;
	bool bCompressionEnableDDC = false;
};

struct FIoContainerSettings
{
	FIoContainerId ContainerId;
	EIoContainerFlags ContainerFlags = EIoContainerFlags::None;
	FGuid EncryptionKeyGuid;
	FAESKey EncryptionKey;
	void* SigningKey;
	bool bGenerateDiffPatch = false;

	bool IsCompressed() const
	{
		return !!(ContainerFlags & EIoContainerFlags::Compressed);
	}

	bool IsEncrypted() const
	{
		return !!(ContainerFlags & EIoContainerFlags::Encrypted);
	}

	bool IsSigned() const
	{
		return !!(ContainerFlags & EIoContainerFlags::Signed);
	}

	bool IsIndexed() const
	{
		return !!(ContainerFlags & EIoContainerFlags::Indexed);
	}
};

struct FIoStoreTocResource
{
	enum { CompressionMethodNameLen = 32 };

	FIoStoreTocHeader Header;
	std::vector<FIoChunkId> ChunkIds;
	std::vector<FIoOffsetAndLength> ChunkOffsetLengths;
	std::vector<int32_t> ChunkPerfectHashSeeds;
	std::vector<int32_t> ChunkIndicesWithoutPerfectHash;
	std::vector<FIoStoreTocCompressedBlockEntry> CompressionBlocks;
	std::vector<std::string> CompressionMethods;
	FSHAHash SignatureHash;
	std::vector<FIoStoreTocEntryMeta> ChunkMetas;
	std::vector<uint8_t> DirectoryIndexBuffer;

	static ReadStatus Read(const char* TocFilePath, EIoStoreTocReadOptions ReadOptions, FIoStoreTocResource& OutTocResource);
	static uint64_t Write(const char* TocFilePath, FIoStoreTocResource& TocResource, const FIoContainerSettings& ContainerSettings, const FIoStoreWriterSettings& WriterSettings);
	static uint64_t HashChunkIdWithSeed(int32_t Seed, const FIoChunkId& ChunkId);
};

static FIoChunkId CreateIoChunkId(uint64_t ChunkId, uint16_t ChunkIndex, EIoChunkType IoChunkType)
{
	uint8_t Data[12] = { 0 };

	*reinterpret_cast<uint64_t*>(&Data[0]) = ChunkId;
	*reinterpret_cast<uint16_t*>(&Data[8]) = NETWORK_ORDER16(ChunkIndex);
	*reinterpret_cast<uint8_t*>(&Data[11]) = static_cast<uint8_t>(IoChunkType);

	FIoChunkId IoChunkId;
	IoChunkId.Set(Data, 12);

	return IoChunkId;
}

class FIoBuffer
{
public:
	enum EAssumeOwnershipTag { AssumeOwnership };
	enum ECloneTag { Clone };
	enum EWrapTag { Wrap };

	FIoBuffer();
	explicit FIoBuffer(uint64_t InSize);
	FIoBuffer(const void* Data, uint64_t InSize, const FIoBuffer& OuterBuffer);
	FIoBuffer(std::span<uint8_t> Memory, const FIoBuffer& OuterBuffer);
	FIoBuffer(EAssumeOwnershipTag, const void* Data, uint64_t InSize);
	FIoBuffer(EAssumeOwnershipTag, std::span<uint8_t> Memory);
	FIoBuffer(ECloneTag, const void* Data, uint64_t InSize);
	FIoBuffer(ECloneTag, std::span<uint8_t> Memory);
	FIoBuffer(EWrapTag, const void* Data, uint64_t InSize);
	FIoBuffer(EWrapTag, std::span<uint8_t> Memory);

	inline const uint8_t* Data() const { return CorePtr->Data(); }
	inline uint8_t* Data() { return CorePtr->Data(); }
	inline const uint8_t* GetData() const { return CorePtr->Data(); }
	inline uint8_t* GetData() { return CorePtr->Data(); }
	inline uint64_t DataSize() const { return CorePtr->DataSize(); }
	inline uint64_t GetSize() const { return CorePtr->DataSize(); }

	inline std::span<uint8_t> GetView() const { return std::span<uint8_t>{ CorePtr->Data(), CorePtr->DataSize() }; }
	inline void	SetSize(uint64_t InSize) { return CorePtr->SetSize(InSize); }
	inline bool	IsMemoryOwned() const { return CorePtr->IsMemoryOwned(); }
	inline void	EnsureOwned() const { if (!CorePtr->IsMemoryOwned()) { MakeOwned(); } }

	void MakeOwned() const;
	uint8_t* Release();

private:
	struct BufCore
	{
		BufCore()
		{
		};

		~BufCore();
		explicit BufCore(uint64_t InSize);
		BufCore(const uint8_t* InData, uint64_t InSize, bool InOwnsMemory);
		BufCore(const uint8_t* InData, uint64_t InSize, const BufCore* InOuter);
		BufCore(ECloneTag, uint8_t* InData, uint64_t InSize);

		BufCore(const BufCore& Rhs) = delete;

		BufCore& operator=(const BufCore& Rhs) = delete;

		inline uint8_t* Data() { return DataPtr; }
		inline uint64_t DataSize() const { return DataSizeLow | (uint64_t(DataSizeHigh) << 32); }

		void SetDataAndSize(const uint8_t* InData, uint64_t InSize);
		void SetSize(uint64_t InSize);
		void MakeOwned();

		uint8_t* ReleaseMemory();

		inline void SetIsOwned(bool InOwnsMemory)
		{
			if (InOwnsMemory)
			{
				Flags |= OwnsMemory;
			}
			else
			{
				Flags &= ~OwnsMemory;
			}
		}

		inline uint32_t AddRef() const;
		inline uint32_t Release() const;

		uint32_t GetRefCount() const
		{
			return uint32_t(NumRefs);
		}

		bool IsMemoryOwned() const { return Flags & OwnsMemory; }

	private:
		void CheckRefCount() const;

		uint8_t* DataPtr = nullptr;
		uint32_t DataSizeLow = 0;
		mutable int32_t NumRefs = 0;

		std::shared_ptr<const BufCore> OuterCore;
		uint8_t	DataSizeHigh = 0;
		uint8_t	Flags = 0;

		enum
		{
			OwnsMemory = 1 << 0,
			ReadOnlyBuffer = 1 << 1,
			FlagsMask = (1 << 2) - 1
		};

		void EnsureDataIsResident() {}

		void ClearFlags()
		{
			Flags = 0;
		}
	};

	std::shared_ptr<BufCore> CorePtr;
};