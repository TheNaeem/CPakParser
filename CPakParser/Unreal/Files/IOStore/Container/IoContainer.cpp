#include "IoContainer.h"

#include "Serialization/Impl/FileReader.h"
#include <filesystem>

bool FFileIoStoreContainerFilePartition::OpenContainer(const char* ContainerFilePath)
{
	FileSize = std::filesystem::file_size(ContainerFilePath);

	if (FileSize < 0)
	{
		return false;
	}

	Ar = std::make_unique<FFileReader>(ContainerFilePath);

	return true;
}