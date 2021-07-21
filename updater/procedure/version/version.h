#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <common/trace.h>
#include <common/json.h>
#include <common/types.h>

enum version_error_e {
    VersionOk,
    VersionNotFound,
    VersionInvalidStringParse,
    VersionInvalidFilePaths,
    VersionInvalidVersionJson,
    VersionInvalidNumber,
    VersionAllocError,
};

bool version_check(trace_list_t *tl, verify_file_handle_s *handle);

const char *strerror_version(int err);

const char *strerror_version_ext(int err, int err_ext);

#ifdef __cplusplus
}
#endif
