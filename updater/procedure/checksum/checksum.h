#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdbool.h>
#include <common/trace.h>

enum checksum_error_e {
    ChecksumOk,
    ChecksumInvalid,
    ChecksumInvalidFilePaths,
    ChecksumJsonFileTooBig,
    ChecksumJsonReadFailed,
    ChecksumJsonParseFailed,
    ChecksumNotFoundInJson,
};

/// all input data required for checksum
struct checksum_handle_s {
    const char *file_to_verify;
    const char *file_version_json;
};

bool checksum_verify(struct checksum_handle_s *handle, trace_list_t *tl);

const char *strerror_checksum(int err);
const char *strerror_checksum_ext(int err, int err_ext);

#ifdef __cplusplus
}
#endif
