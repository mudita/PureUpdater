#include <errno.h>
#include <stdlib.h>
#include <common/boot_files.h>
#include <common/path_opts.h>

#include "version.h"
#include "version_priv.h"

#define UNUSED(expr) do { (void)(expr); } while (0)

bool version_check_all(trace_list_t *tl, verify_file_handle_s *handle, const char *tmp_path){
    bool ret = true;

    for (size_t i = 0; i < verify_files_list_size; ++i) {
        const char *filename = verify_files[i];
        char *filepath  = (char *)calloc(1,strlen(filename) + strlen(tmp_path) + 1);
        sprintf(filepath, "%s/%s", tmp_path, filename);
        if (!path_check_if_exists(filepath)) {
            free(filepath);
            break;
        }
        handle->file_to_verify = filepath;
        ret = version_check(tl, handle);
        free(filepath);
        if(!ret){
            return ret;
        }
    }

    return ret;
}

bool version_check(trace_list_t *tl, verify_file_handle_s *handle) {
    bool ret = false;
    char trace_title[30];

    if (tl == NULL || handle == NULL) {
        printf("version_verify trace/handle null error");
        goto exit;
    }

    sprintf(trace_title, "%s:%s", __func__, handle->file_to_verify);
    trace_t *trace = trace_append(trace_title, tl, strerror_version, NULL);

    if (handle->version_json.valid == false) {
        trace_write(trace, VersionInvalidVersionJson, errno);
        goto exit;
    }

    if (handle->file_to_verify == NULL) {
        trace_write(trace, VersionInvalidFilePaths, errno);
        goto exit;
    }

    version_s new_file_version = version_get(trace, &handle->version_json, handle->file_to_verify);
    if (!new_file_version.valid) {
        trace_write(trace, VersionInvalidStringParse, errno);
        goto exit;
    }

    version_s current_file_version = version_get(trace, &handle->current_version_json, handle->file_to_verify);
    if (!current_file_version.valid) {
        trace_write(trace, VersionInvalidStringParse, errno);
        goto exit;
    }

    if (version_is_lhs_newer(&new_file_version, &current_file_version)) {
        ret = true;
    }

    exit:
    return ret;
}

const char *strerror_version(int err) {
    switch (err) {
        case VersionOk:
            return "VersionOk";
        case VersionNotFound:
            return "VersionNotFound";
        case VersionInvalidStringParse:
            return "VersionInvalidStringParse";
        case VersionAllocError:
            return "VersionAllocError";
    }
    return "Unknown";
}

const char *strerror_version_ext(int err, int err_ext) {
    UNUSED(err);
    UNUSED(err_ext);
    return "Unknown";
}
