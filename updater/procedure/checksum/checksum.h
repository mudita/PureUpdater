#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <common/trace.h>
#include <common/json.h>
#include <common/types.h>

enum checksum_error_e {
    ChecksumOk,
    ChecksumInvalid,
    ChecksumInvalidFilePaths,
    ChecksumJsonFileTooBig,
    ChecksumJsonReadFailed,
    ChecksumJsonParseFailed,
    ChecksumNotFoundInJson,
    ChecksumInvalidVersionJson,
    ChecksumGenericError,
};

bool checksum_verify_all(trace_list_t *tl, verify_file_handle_s *handle, const char *tmp_path);

bool checksum_verify(trace_list_t *tl, verify_file_handle_s *handle);

const char *strerror_checksum(int err);

const char *strerror_checksum_ext(int err, int err_ext);

#ifdef __cplusplus
}
#endif
