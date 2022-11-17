#include "IoContainer.h"

#include <filesystem>

import CPakParser.Serialization.FileReader;

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