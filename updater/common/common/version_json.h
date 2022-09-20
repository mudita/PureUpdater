#pragma once

#include <common/log.h>
#include <cJSON/cJSON.h>
#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif


version_json_s json_get_version_struct(const char *json_path);

version_json_file_s json_get_file_from_version(const version_json_s *version_json, const char *name);

/// get version json for current file and for curent release in use
/// if there is no version.json for curent release - generate fallback version.json values
/// if any of values in return struct are set valid = false - user should fail procedure
verify_file_handle_s json_get_verify_files(const char *new_version, const char *current_version);

#ifdef __cplusplus
}
#endif
