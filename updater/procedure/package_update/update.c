#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <hal/security.h>
#include <hal/hwcrypt/signature.h>
#include "common/log.h"
#include "update.h"
#include "priv_update.h"
#include "priv_tmp.h"
#include "procedure/checksum/checksum.h"
#include "procedure/version/version.h"
#include "procedure/backup/backup.h"
#include "procedure/package_update/update_ecoboot.h"
#include <procedure/security/pgmkeys.h>

static void str_clean_up(char **str) {
    if (str) {
        free(*str);
    }
}

static void verify_file_handle_cleanup(verify_file_handle_s *handle) {
    if (handle) {
        if (handle->current_version_json.boot.md5sum)
            free(handle->current_version_json.boot.md5sum);
        if (handle->current_version_json.boot.name)
            free(handle->current_version_json.boot.name);
        if (handle->current_version_json.boot.version)
            free(handle->current_version_json.boot.version);

        if (handle->version_json.boot.md5sum)
            free(handle->version_json.boot.md5sum);
        if (handle->version_json.boot.name)
            free(handle->version_json.boot.name);
        if (handle->version_json.boot.version)
            free(handle->version_json.boot.version);
    }
}

static int signature_check(const char *name) {
    if (sec_configuration_is_open()) {
        return sec_verify_ok;
    }
    const char sig_ext[] = ".sig";
    char *signature_name __attribute__((__cleanup__(str_clean_up))) = malloc(strlen(name) + sizeof(sig_ext));
    strcpy(signature_name, name);
    strcat(signature_name, sig_ext);
    return sec_verify_file(name, signature_name);
}

void update_firmware_init(struct update_handle_s *h) {
    memset(h, 0, sizeof *h);
}

// Program the keys if it is needed
static void program_secure_fuses(const struct update_handle_s *handle) {
    const char key_fn[] = "/SRK_fuses.bin";
    const char sum_fn[] = "/SRK_fuses.bin.md5";
    struct program_keys_handle khandle;
    khandle.srk_file = malloc(strlen(handle->tmp_os) + sizeof(key_fn));
    khandle.chksum_srk_file = malloc(strlen(handle->tmp_os) + sizeof(sum_fn));
    strcpy(khandle.srk_file, handle->tmp_os);
    strcat(khandle.srk_file, key_fn);
    strcpy(khandle.chksum_srk_file, handle->tmp_os);
    strcat(khandle.chksum_srk_file, sum_fn);
    if (program_keys_is_needed(&khandle)) {
        debug_log("Update: keys programming is required. Key file: %s checksum: %s", khandle.srk_file,
                  khandle.chksum_srk_file);
        if (program_keys(&khandle)) {
            debug_log("Update: failed to program keys");
        }
        unlink(khandle.srk_file);
        unlink(khandle.chksum_srk_file);
        free(khandle.srk_file);
        free(khandle.chksum_srk_file);
    } else {
        debug_log("Update: programming the keys is not needed");
    }
}

bool update_firmware(struct update_handle_s *handle) {
    debug_log("Starting firmware update");
    bool success = false;
    struct backup_handle_s backup_handle = {
            .backup_from_os = handle->update_os,
            .backup_from_user = handle->update_user,
            .backup_to = handle->backup_full_path
    };
    if (handle->enabled.check_sign) {
        debug_log("Update: signature check");
        const int err = signature_check(handle->update_from);
        if (err) {
            handle->unsigned_tar = true;
        } else {
            handle->unsigned_tar = false;
        }
        debug_log("Update: package is signed: %s", handle->unsigned_tar ? "FALSE" : "TRUE");
    } else {
        debug_log("Update: package signature check skipped");
    }

    if (handle->enabled.backup) {
        debug_log("Update: performing backup");
        if (handle->enabled.backup && !backup_previous_firmware(&backup_handle)) {
            debug_log("Update: backup error");
            success = false;
            goto exit;
        }
    }

    debug_log("Update: setup temporary catalog");
    if (!tmp_create_catalog(handle)) {
        debug_log("Update: tmp setup failed");
        success = false;
        goto exit;
    }

    debug_log("Update: unpacking update archive");
    if (!unpack(handle)) {
        debug_log("Update: unpacking error");
        success = false;
        goto exit;
    }

    if (handle->enabled.check_checksum || handle->enabled.check_version) {
        debug_log("Update: verify files");
        verify_file_handle_s verify_handle __attribute__((__cleanup__(verify_file_handle_cleanup))) =
                json_get_verify_files(handle->new_version_json, handle->current_version_json);

        if (handle->enabled.check_checksum) {
            debug_log("Update: verify checksum");
            if (!checksum_verify_all(&verify_handle, handle->tmp_os)) {
                debug_log("Update: checksum mismatch!");
                success = false;
                goto exit;
            }
        }
        if (handle->enabled.check_version) {
            debug_log("Update: verify versions");
            if (!version_check_all(&verify_handle, handle->tmp_os, handle->enabled.allow_downgrade)) {
                debug_log("Update: verify version failed");
                success = false;
                goto exit;
            }
        }
    }

    debug_log("Update: program fuses");
    program_secure_fuses(handle);

    debug_log("Update: moving files from tmp to destination");
    if (!tmp_files_move(handle)) {
        debug_log("Update: moving error");
        success = false;
        goto exit;
    }

    // Finally update the ecoboot bin
    int ecoboot_package_status = ecoboot_in_package(handle->update_os, ecoboot_filename);
    if (ecoboot_package_status == 1) {
        debug_log("Update: updating %s",ecoboot_filename);
        const int eco_status = ecoboot_update(handle->update_os, ecoboot_filename);
        if (eco_status != error_eco_update_ok) {
            if (eco_status != error_eco_vfs && errno != ENOENT) {
                debug_log("Update: %s update error, errno: %d",ecoboot_filename, errno);
                success = false;
                goto exit;
            }
        } else {
            debug_log("Update: %s updated successfully",ecoboot_filename);
        }
    } else {
        debug_log("Update: No %s in package",ecoboot_filename);
    }
    success = true;
    exit:
    return success;
}
