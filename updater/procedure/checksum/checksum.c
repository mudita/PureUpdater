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

bool checksum_verify_all(verify_file_handle_s *handle, const char *tmp_path) {
    bool ret = true;
    debug_log("Checksum: verifying all files");

    for (size_t i = 0; i < verify_files_list_size; ++i) {
        const char *filename = verify_files[i];
        char *filepath = (char *) calloc(1, strlen(filename) + strlen(tmp_path) + 1);
        sprintf(filepath, "%s/%s", tmp_path, filename);
        if (!path_check_if_exists(filepath)) {
            free(filepath);
            break;
        }
        handle->file_to_verify = filepath;
        ret = checksum_verify(handle);
        free(filepath);
        if (!ret) {
            return ret;
        }
    }
    return ret;
}

bool checksum_verify(verify_file_handle_s *handle) {
    unsigned char calculated_checksum[16];
    char calculated_checksum_readable[33];

    debug_log("Checksum: verifying file: %s", handle->file_to_verify);

    int file_to_verify_fd __attribute__((__cleanup__(_autoclose))) = -1;
    bool ret = false;

    if (handle == NULL) {
        debug_log("Checksum: failed to open file to verify checksum");
        goto exit;
    }

    if (handle->version_json.valid == false) {
        debug_log("Checksum: version.json is not valid");
        goto exit;
    }

    if (handle->file_to_verify == NULL) {
        debug_log("Checksum: file to verify is a NULL");
        goto exit;
    }
    file_to_verify_fd = open(handle->file_to_verify, O_RDONLY);
    if (file_to_verify_fd <= 0) {
        debug_log("Checksum: failed to open the file");
        goto exit;
    }

    MD5_File(calculated_checksum, handle->file_to_verify);
    checksum_get_readable(calculated_checksum, calculated_checksum_readable);

    version_json_file_s file_version = json_get_file_from_version(&handle->version_json, handle->file_to_verify);
    if (file_version.valid == false) {
        debug_log("Checksum: checksum for file not found in version.json");
        goto exit;
    }

    ret = checksum_compare(file_version.md5sum, calculated_checksum_readable);
    if (!ret) {
        debug_log("Checksum: checksum mismatch for file:%s (%s : %s)", handle->file_to_verify, file_version.md5sum,
                  calculated_checksum_readable);
        goto exit;
    }

    exit:
    return ret;
}
