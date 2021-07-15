#pragma once

#include <common/trace.h>
#include <cJSON/cJSON.h>

#ifdef __cplusplus
extern "C"
{
#endif

const char *get_checksum(trace_list_t *tl, const cJSON *json, const char *file_name);
cJSON *get_json(trace_list_t *tl, const char *json_path);
bool compare_checksums(const char *checksum_l, const char *checksum_r);

#ifdef __cplusplus
}
#endif
