#include "Dataminer.h"

#include "Core/Globals/GlobalContext.h"
#include "Core/Reflection/Mappings.h"

import CPakParser.Files.Loader;
import CPakParser.Serialization.FArchive;

bool Dataminer::LoadTypeMappings(std::string UsmapFilePath)
{
	return Mappings::RegisterTypesFromUsmap(UsmapFilePath, Context->ObjectArray);
}

std::future<bool> Dataminer::LoadTypeMappingsAsync(std::string UsmapFilePath)
{
	return std::async(std::launch::async, &Dataminer::LoadTypeMappings, this, UsmapFilePath);
}

void Dataminer::SerializeFileInternal(FGameFilePath& FilePath, TSharedPtr<ISerializableFile> OutFile)
{
	auto Entry = Context->FilesManager.FindFile(FilePath);
	auto Reader = FLoader::CreateFileReader(Entry);

	if (!Reader)
		return;

	OutFile->Serialize(*Reader);
}

void Dataminer::Test(FGameFilePath Path)
{
	auto Entry = Context->FilesManager.FindFile(Path);

	if (!Entry.IsValid())
		return;

	auto Reader = FLoader::CreateFileReader(Entry);

	Entry.GetAssociatedFile()->DoWork(Reader, Context); // TODO: wrap UPackage around this
}