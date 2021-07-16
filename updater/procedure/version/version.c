#include <errno.h>

#include "version.h"
#include "version_priv.h"

#define UNUSED(expr) do { (void)(expr); } while (0)

bool version_verify(trace_list_t *tl, verify_file_handle_s *handle) {
    bool ret = false;

    if (tl == NULL || handle == NULL) {
        printf("version_verify trace/handle null error");
        goto exit;
    }
    trace_t *trace = trace_append("version_verify", tl, strerror_version, NULL);

    if (handle->version_json.valid == false) {
        trace_write(trace, VersionInvalidVersionJson, errno);
        goto exit;
    }

    if (handle->file_to_verify == NULL) {
        trace_write(trace, VersionInvalidFilePaths, errno);
        goto exit;
    }

    version_s new_file_version = version_get(tl, &handle->version_json, handle->file_to_verify);
    if (!new_file_version.valid) {
        trace_write(trace, VersionInvalidStringParse, errno);
        goto exit;
    }

    version_s current_file_version = version_get(tl, &handle->current_version_json, handle->file_to_verify);
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
