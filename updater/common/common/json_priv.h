#pragma once

#include "json.h"

#ifdef __cplusplus
extern "C"
{
#endif

cJSON *json_get(trace_t *trace, const char *json_path);

cJSON *json_get_item_from(trace_t *trace, const cJSON *json, const char *name);

version_json_file_s json_get_file_struct(trace_t *trace, const cJSON *json, const char *filename_arg);

#ifdef __cplusplus
}
#endif
