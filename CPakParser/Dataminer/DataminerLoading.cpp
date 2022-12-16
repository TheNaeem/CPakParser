#include "Dataminer.h"

#include "Core/Globals/GlobalContext.h"
#include "Core/Reflection/Mappings.h"

import CPakParser.Serialization.FArchive;
import CPakParser.Files.SerializableFile;
import CPakParser.Logging;

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
	auto Reader = Entry.CreateReader();

	if (!Reader)
		return;

	OutFile->Serialize(*Reader);
}

UPackagePtr Dataminer::LoadPackage(FGameFilePath Path)
{
	auto Entry = Context->FilesManager.FindFile(Path);

	if (!Entry.IsValid())
		return nullptr;

	auto Reader = Entry.CreateReader();

	if (!Reader)
	{
		LogError("Could not create reader for %s", Path.FileName.c_str());
		return nullptr;
	}

	return Entry.GetAssociatedFile()->CreatePackage(*Reader, Context);
}

UObjectPtr Dataminer::LoadObject(FGameFilePath Path)
{
	auto Package = LoadPackage(Path);

	if (!Package)
		return nullptr;

	// TODO ASAP: get export name from FGameFilePath if there is one provided
	return Package->GetExportByName(Package->GetName());
}