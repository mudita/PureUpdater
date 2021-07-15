#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <md5/md5.h>

#include "checksum.h"
#include "priv_checksum.h"


static void _autoclose(int *f) {
    if (*f > 0) {
        close(*f);
    }
}

#define UNUSED(expr) do { (void)(expr); } while (0)
#define AUTOCLOSE(var) int var __attribute__((__cleanup__(_autoclose)))

bool checksum_verify(struct checksum_handle_s *handle, trace_list_t *tl) {
    const size_t checksum_size = 16;
    unsigned char file_md5_checksum[checksum_size];
    bool ret = false;

    cJSON *json = NULL;
    char *checksum = NULL;

    if (tl == NULL || handle == NULL) {
        printf("checksum_verify trace/handle null error");
        goto exit;
    }

    trace_t *trace = trace_append("checksum_verify", tl, strerror_checksum, strerror_checksum_ext);

    if (handle->file_to_verify == NULL || handle->file_version_json == NULL) {
        trace_write(trace, ChecksumInvalidFilePaths, errno);
        goto exit;
    }

    AUTOCLOSE(file_to_verify_fd) = open(handle->file_to_verify, O_RDONLY);
    if (file_to_verify_fd <= 0) {
        trace_write(trace, ChecksumInvalidFilePaths, errno);
        trace_printf(trace, handle->file_to_verify);
        goto exit;
    }

    MD5_File(file_md5_checksum, handle->file_to_verify);

    json = get_json(tl, handle->file_version_json);
    if (json == NULL) {
        trace_write(trace, ChecksumInvalidFilePaths, errno);
        trace_printf(trace, handle->file_version_json);
        goto exit;
    }

    checksum = (char *) get_checksum(tl, json, handle->file_to_verify);
    if (checksum == NULL) {
        trace_write(trace, ChecksumNotFoundInJson, errno);
        goto exit;
    }

    ret = compare_checksums(checksum, (char *) file_md5_checksum);

    exit:
    cJSON_Delete(json);
    free(checksum);
    return ret;
}

const char *strerror_checksum(int err) {
    switch (err) {
        case ChecksumOk:
            return "ChecksumOk";
        case ChecksumInvalid:
            return "ChecksumInvalid";
        case ChecksumInvalidFilePaths:
            return "ChecksumInvalidFilePaths";
        case ChecksumJsonFileTooBig:
            return "ChecksumJsonFileTooBig";
        case ChecksumJsonReadFailed:
            return "ChecksumJsonReadFailed";
        case ChecksumJsonParseFailed:
            return "ChecksumJsonParseFailed";
        case ChecksumNotFoundInJson:
            return "ChecksumNotFoundInJson";
    }
    return "Unknown";
}

const char *strerror_checksum_ext(int err, int err_ext) {
    switch(err)
    {
        case ChecksumJsonReadFailed:
        case ChecksumJsonFileTooBig:
        case ChecksumInvalidFilePaths:
        case ChecksumNotFoundInJson:
            return strerror(err_ext);
    }
    return "";
}
