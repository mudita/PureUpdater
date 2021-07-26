#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <md5/md5.h>
#include <common/boot_files.h>
#include <common/path_opts.h>

#include "checksum.h"
#include "checksum_priv.h"

static void _autoclose(int *f) {
    if (*f > 0) {
        close(*f);
    }
}

#define UNUSED(expr) do { (void)(expr); } while (0)
#define AUTOCLOSE(var) int var __attribute__((__cleanup__(_autoclose)))

bool checksum_verify_all(trace_list_t *tl, verify_file_handle_s *handle, const char *tmp_path) {
    bool ret = true;

    for (size_t i = 0; i < verify_files_list_size; ++i) {
        const char *filename = verify_files_list[i];
        char *filepath = (char *) calloc(1, strlen(filename) + strlen(tmp_path) + 1);
        sprintf(filepath, "%s/%s", tmp_path, filename);
        if (!path_check_if_exists(filepath)) {
            free(filepath);
            break;
        }
        handle->file_to_verify = filepath;
        ret = checksum_verify(tl, handle);
        free(filepath);
        if (!ret) {
            return ret;
        }
    }
    return ret;
}

bool checksum_verify(trace_list_t *tl, verify_file_handle_s *handle) {
    unsigned char calculated_checksum[16];
    char calculated_checksum_readable[33];
    char trace_title[30];

    bool ret = false;

    if (tl == NULL || handle == NULL) {
        printf("checksum_verify trace/handle null error");
        goto exit;
    }

    sprintf(trace_title, "%s:%s", __func__, handle->file_to_verify);
    trace_t *trace = trace_append(trace_title, tl, strerror_checksum, NULL);

    if (handle->version_json.valid == false) {
        trace_write(trace, ChecksumInvalidVersionJson, errno);
        goto exit;
    }

    if (handle->file_to_verify == NULL) {
        trace_write(trace, ChecksumInvalidFilePaths, errno);
        trace_printf(trace, handle->file_to_verify);
        goto exit;
    }

    AUTOCLOSE(file_to_verify_fd) = open(handle->file_to_verify, O_RDONLY);
    if (file_to_verify_fd <= 0) {
        trace_write(trace, ChecksumInvalidFilePaths, errno);
        trace_printf(trace, handle->file_to_verify);
        goto exit;
    }

    MD5_File(calculated_checksum, handle->file_to_verify);
    checksum_get_readable(calculated_checksum, calculated_checksum_readable);

    version_json_file_s file_version = json_get_file_from_version(trace, &handle->version_json, handle->file_to_verify);
    if (file_version.valid == false) {
        trace_write(trace, ChecksumNotFoundInJson, errno);
        goto exit;
    }

    ret = checksum_compare(file_version.md5sum, calculated_checksum_readable);
    if (!ret) {
        trace_write(trace, ChecksumInvalid, errno);
        goto exit;
    }

    exit:
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
    switch (err) {
        case ChecksumJsonReadFailed:
        case ChecksumJsonFileTooBig:
        case ChecksumInvalidFilePaths:
        case ChecksumNotFoundInJson:
            return strerror(err_ext);
    }
    return "";
}
