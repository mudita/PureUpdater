#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdbool.h>
#include <common/trace.h>
#include <cJSON/cJSON.h>

enum checksum_error_e {
    ChecksumOk,
    ChecksumInvalid,
    ChecksumInvalidFilePaths,
    ChecksumJsonFileTooBig,
    ChecksumJsonReadFailed,
    ChecksumJsonParseFailed,
    ChecksumNotFoundInJson,
    ChecksumGenericError,
};

/// all input data required for checksum
struct checksum_handle_s {
    const char *file_to_verify;
    const char *file_version_json;
};

bool checksum_verify(struct checksum_handle_s *handle, trace_list_t *tl);

const char *strerror_checksum(int err);

const char *strerror_checksum_ext(int err, int err_ext);

cJSON *get_json(trace_list_t *tl, const char *json_path);

const char *get_checksum(trace_list_t *tl, const cJSON *json, const char *file_name);

bool compare_checksums(const char *checksum_l, const char *checksum_r);

#ifdef __cplusplus
}
#endif
