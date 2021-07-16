#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <md5/md5.h>

#include "checksum.h"

const char *checksum_label = "checksums";

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

    trace_t *trace = trace_append("checksum_verify", tl, strerror_checksum, NULL);

    if (handle->file_to_verify == NULL || handle->file_version_json == NULL) {
        trace_write(trace, ChecksumInvalidFilePaths, errno);
        goto exit;
    }

    AUTOCLOSE(file_to_verify_fd) = open(handle->file_to_verify, O_RDONLY);
    if (file_to_verify_fd <= 0) {
        trace_write(trace, ChecksumInvalidFilePaths, errno);
        goto exit;
    }

    MD5_File(file_md5_checksum, handle->file_to_verify);

    json = get_json(tl, handle->file_version_json);
    if (json == NULL) {
        trace_write(trace, ChecksumInvalidFilePaths, errno);
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
        case ChecksumGenericError:
            return "ChecksumGenericError";
    }
    return "Unknown";
}

const char *strerror_checksum_ext(int err, int err_ext) {
    UNUSED(err);
    UNUSED(err_ext);
    return "Unknown";
}

const char *get_checksum(trace_list_t *tl, const cJSON *json, const char *file_name) {
    const cJSON *checksums_tab = NULL;
    const cJSON *file_checksum = NULL;

    if (tl == NULL) {
        goto exit;
    }

    trace_t *trace = trace_append("get_checksum", tl, strerror_checksum, NULL);

    if (json != NULL && file_name != NULL) {
        checksums_tab = get_from_json(tl, json, checksum_label);
        file_checksum = get_from_json(tl, checksums_tab, file_name);
        if (file_checksum != NULL) {
            return file_checksum->valuestring;
        } else {
            trace_write(trace, ChecksumNotFoundInJson, errno);
            goto exit;
        }
    }

    exit:
    return NULL;
}

bool compare_checksums(const char *checksum_l, const char *checksum_r) {
    return (strcmp(checksum_l, checksum_r) == 0);
}
