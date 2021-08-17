#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <common/enum_s.h>
#include "common/trace.h"
#include "update.h"
#include "priv_update.h"
#include "priv_tmp.h"
#include "procedure/checksum/checksum.h"
#include "procedure/version/version.h"
#include "procedure/backup/backup.h"
#include "procedure/package_update/update_ecoboot.h"

const char *update_strerror(int e)
{
    switch (e)
    {
        ENUMS(ErrorUpdateOk);
        ENUMS(ErrorSignCheck);
        ENUMS(ErrorUnpack);
        ENUMS(ErrorBackup);
        ENUMS(ErrorTmp);
        ENUMS(ErrorChecksums);
        ENUMS(ErrorVersion);
        ENUMS(ErrorMove);
        ENUMS(ErrorUpdateEcoboot);
    }
    return "UNKNOWN";
}

bool signature_check(const char *name, trace_list_t *tl)
{
    (void)name;
    (void)tl;
    return false;
}

void update_firmware_init(struct update_handle_s *h)
{
    memset(h, 0, sizeof *h);
}

bool update_firmware(struct update_handle_s *handle, trace_list_t *tl)
{
    trace_t *t = trace_append("update", tl, update_strerror, NULL);

    struct backup_handle_s backup_handle = {.backup_from_os = handle->update_os,
                                            .backup_from_user = handle->update_user,
                                            .backup_to = handle->backup_full_path};
    if (handle->enabled.check_sign)
    {
        if (!signature_check(handle->update_from, tl))
        {
            trace_write(t, ErrorSignCheck, 0);
            goto exit;
        }
    }

    if (handle->enabled.backup)
    {
        if (handle->enabled.backup && !backup_previous_firmware(&backup_handle, tl))
        {
            trace_write(t, ErrorBackup, 0);
            goto exit;
        }
    }

    if (!tmp_create_catalog(handle, tl))
    {
        trace_write(t, ErrorBackup, 0);
        goto exit;
    }

    if (!unpack(handle, tl))
    {
        trace_write(t, ErrorUnpack, 0);
        goto exit;
    }

    if (handle->enabled.check_checksum || handle->enabled.check_version) {
        verify_file_handle_s verify_handle = json_get_verify_files(t, "/os/tmp/version.json", "/os/current/version.json");

        if (handle->enabled.check_checksum) {
            if (!checksum_verify_all(tl, &verify_handle, handle->tmp_os)) {
                trace_write(t, ErrorChecksums, 0);
                goto exit;
            }
        }
        if (handle->enabled.check_version) {
            if (!version_check_all(tl, &verify_handle, handle->tmp_os)) {
                trace_write(t, ErrorVersion, 0);
                goto exit;
            }
        }
    }

    if (!tmp_files_move(handle, tl))
    {
        trace_write(t, ErrorMove, 0);
        goto exit;
    }

    // Finally update the ecoboot bin
    int ecoboot_package_status = ecoboot_in_package(handle->update_os, "ecoboot.bin");
    if (ecoboot_package_status == 1) {
        const int eco_status = ecoboot_update(handle->update_os, "ecoboot.bin", tl);
        if (eco_status != error_eco_update_ok) {
            if (eco_status != error_eco_vfs && errno != ENOENT) {
                trace_write(t, ErrorUpdateEcoboot, 0);
                goto exit;
            }
        }
        else {
            printf("Ecoboot update success\n");
        }
    }
    else {
        printf("No ecoboot.bin in package, %d\n", ecoboot_package_status);
    }

exit:
    return trace_ok(t);
}
