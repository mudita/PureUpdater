#include <errno.h>
#include <stdlib.h>
#include <common/boot_files.h>
#include <common/path_opts.h>

#include "version.h"
#include "version_priv.h"

static void version_cleanup(version_s *handle) {
    if (handle && handle->str) {
        free(handle->str);
    }
}

bool version_check_all(verify_file_handle_s *handle, const char *tmp_path, bool allow_downgrade) {
    bool ret = true;

    for (size_t i = 0; i < verify_files_list_size; ++i) {
        const char *filename = verify_files[i];
        char *filepath = NULL;
        asprintf(&filepath, "%s/%s", tmp_path, filename);
        if (!filepath || !path_check_if_exists(filepath)) {
            free(filepath);
            break;
        }
        handle->file_to_verify = filepath;
        ret = version_check(handle, allow_downgrade);
        free(filepath);
        if (!ret) {
            return ret;
        }
    }

    return ret;
}

bool version_check(verify_file_handle_s *handle, bool allow_downgrade) {
    bool ret = false;

    if (handle == NULL) {
        debug_log("Version: handle is null");
        goto exit;
    }

    if (handle->version_json.valid == false) {
        debug_log("Version: version.json is not valid");
        goto exit;
    }

    if (handle->file_to_verify == NULL) {
        debug_log("Version: file to verify is null");
        goto exit;
    }

    version_s new_file_version __attribute__((__cleanup__(version_cleanup))) =
            version_get(&handle->version_json, handle->file_to_verify);

    if (!new_file_version.valid) {
        debug_log("Version: version is not valid");
        goto exit;
    }

    version_s current_file_version __attribute__((__cleanup__(version_cleanup))) =
            version_get(&handle->current_version_json, handle->file_to_verify);

    if (!current_file_version.valid) {
        debug_log("Version: file version is not valid");
        goto exit;
    }

    if (!allow_downgrade && !version_is_lhs_newer(&new_file_version, &current_file_version)) {
        debug_log("Version: downgrade is not allowed");
        goto exit;
    }

    ret = true;

    exit:
    return ret;
}