#pragma once

#include <common/trace.h>
#include <cJSON/cJSON.h>

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

const char *strerror_json(int err);

const char *strerror_json_ext(int err, int err_ext);

cJSON *get_json(trace_list_t *tl, const char *json_path);

cJSON *get_from_json(trace_list_t *tl, const cJSON *json, const char *name);

#ifdef __cplusplus
}
#endif
