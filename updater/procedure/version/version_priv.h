#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <common/trace.h>
#include <common/json.h>
#include <common/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

version_s version_get(trace_t *trace, const version_json_s *version_json, const char *file_name);

const char *version_get_str(version_s *version);

bool version_is_lhs_newer(const version_s *version_l, const version_s *version_r);

int version_parse_str(trace_t *trace, version_s *version, const char *version_str);

#ifdef __cplusplus
}
#endif
