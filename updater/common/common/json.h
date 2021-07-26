#pragma once

#include <common/trace.h>
#include <cJSON/cJSON.h>
#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum json_error_e {
    JsonOk,
    JsonReadFailed,
    JsonFileTooBig,
    JsonParseFailed,
    JsonInvalidFilePaths,
    JsonItemNotFound,
};

version_json_s json_get_version_struct(trace_t *trace, const char *json_path);

version_json_file_s json_get_file_from_version(trace_t *trace, const version_json_s *version_json, const char *name);

const char *strerror_json(int err);

const char *strerror_json_ext(int err, int err_ext);

#ifdef __cplusplus
}
#endif
