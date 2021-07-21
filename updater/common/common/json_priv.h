#pragma once

#include "json.h"

#ifdef __cplusplus
extern "C"
{
#endif

cJSON *json_get(trace_list_t *tl, const char *json_path);

cJSON *json_get_item_from(trace_list_t *tl, const cJSON *json, const char *name);

version_json_file_s json_get_file_struct(trace_list_t *tl, const cJSON *json, const char *filename_arg);

#ifdef __cplusplus
}
#endif
