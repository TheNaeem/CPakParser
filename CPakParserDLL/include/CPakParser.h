#pragma once

#ifdef CPAKPARSERDLL_EXPORTS
#define CPAKPARSERDLL_API __declspec(dllexport)
#else
#define CPAKPARSERDLL_API __declspec(dllimport)
#endif

#ifdef __cplusplus
#include "Dataminer/Dataminer.h"

extern "C" {
#else
typedef void Dataminer;
#endif

CPAKPARSERDLL_API Dataminer* dataminer_construct(const char* PaksFolderDir);
CPAKPARSERDLL_API void dataminer_options_with_oodle(const char* OodleDllPath);
CPAKPARSERDLL_API void dataminer_with_logging(bool bEnableLogging);

CPAKPARSERDLL_API bool dataminer_initialize(Dataminer* This);
CPAKPARSERDLL_API size_t dataminer_files_size(Dataminer* This);
CPAKPARSERDLL_API bool dataminer_load_type_mappings(Dataminer* This, const char* UsmapFilePath);
CPAKPARSERDLL_API void dataminer_set_version_ue5(Dataminer* This, int Version);

#ifdef __cplusplus
}  // extern "C"
#endif