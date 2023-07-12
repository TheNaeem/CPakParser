#include "include/CPakParser.h"

Dataminer* dataminer_construct(const char* PaksFolderDir)
{
    auto Result = std::make_unique<Dataminer>(PaksFolderDir);

    return Result.release();
}

void dataminer_options_with_oodle(const char* OodleDllPath)
{
    Dataminer::Options::WithOodleDecompressor(OodleDllPath);
}

void dataminer_with_logging(bool bEnableLogging)
{
    Dataminer::Options::WithLogging(bEnableLogging);
}

bool dataminer_initialize(Dataminer* This)
{
    return This->Initialize();
}

size_t dataminer_files_size(Dataminer* This)
{
    return This->Files().size();
}

bool dataminer_load_type_mappings(Dataminer* This, const char* UsmapFilePath)
{
    return This->LoadTypeMappings(UsmapFilePath);
}

void dataminer_set_version_ue5(Dataminer* This, int Version)
{
    This->SetVersionUE5(Version);
}