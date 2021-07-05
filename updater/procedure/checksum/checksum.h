#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include "common/trace.h"

    enum backup_error_e
    {
        ErrorChecksumOk,
    };

    /// all input data required for checksum
    struct backup_handle_s
    {
        const char *file_to_verify;
        const char *file_version_json;
    };

    bool checksum_verify(struct backup_handle_s *handle, trace_list_t *tl);
    const char *strerror_checksum(int err);
    const char *strerror_checksum_ext(int err, int err_ext);

#ifdef __cplusplus
}
#endif
