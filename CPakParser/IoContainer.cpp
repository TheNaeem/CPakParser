#include "IoTocResource.h"

void FFileIoStoreContainerFile::GetPartitionFileHandleAndOffset(uint64_t TocOffset, std::shared_ptr<std::ifstream>& OutFileHandle, uint64_t& OutOffset)
{
	int32_t PartitionIndex = int32_t(TocOffset / TocResource->Header.PartitionSize);
	const FFileIoStoreContainerFilePartition& Partition = Partitions[PartitionIndex];
	OutFileHandle = Partition.FileHandle;
	OutOffset = TocOffset % TocResource->Header.PartitionSize;
}