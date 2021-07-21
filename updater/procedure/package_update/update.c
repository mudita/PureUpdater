#include <string.h>
#include <common/enum_s.h>
#include "common/trace.h"
#include "update.h"
#include "priv_update.h"
#include "priv_tmp.h"
#include "procedure/checksum/checksum.h"
#include "procedure/version/version.h"
#include "procedure/backup/backup.h"

const char *update_strerror(int e)
{
    switch(e)
    {
        ENUMS(ErrorUpdateOk);
        ENUMS(ErrorSignCheck);
        ENUMS(ErrorUnpack);
        ENUMS(ErrorBackup);
        ENUMS(ErrorTmp);
        ENUMS(ErrorChecksums);
        ENUMS(ErrorVersion);
        ENUMS(ErrorMove);
    }
    return "UNKNOWN";
}

bool signature_check(const char* name, trace_list_t *tl)
{
    (void)name;
    (void)tl;
    return false;
}

struct version_handle_t{};

void update_firmware_init(struct update_handle_s *h)
{
    memset(h, 0, sizeof *h);
}

bool update_firmware(struct update_handle_s *handle, trace_list_t *tl)
{
    trace_t *t = trace_append("update", tl, update_strerror, NULL);

    struct backup_handle_s backup_handle = {.backup_from_os   = handle->update_os,
                                            .backup_from_user = handle->update_user,
                                            .backup_to        = handle->backup_full_path};

    verify_file_handle_s verify_handle;
    verify_handle.version_json = json_get_version_struct(tl, handle->tmp_os);
    verify_handle.current_version_json = json_get_version_struct(tl, "/os/current/version.json");

    if (handle->enabled.check_sign) {
        if (!signature_check(handle->update_from, tl)) {
            trace_write(t, ErrorSignCheck, 0);
            goto exit;
        }
    }

    if (handle->enabled.backup) {
        if (handle->enabled.backup && !backup_previous_firmware(&backup_handle, tl)) {
            trace_write(t, ErrorBackup, 0);
            goto exit;
        }
    }

    if (!tmp_create_catalog(handle, tl)) {
        trace_write(t, ErrorBackup, 0);
        goto exit;
    }

    if (!unpack(handle, tl)) {
        trace_write(t, ErrorUnpack, 0);
        goto exit;
    }


    if (handle->enabled.check_checksum) {
        /// TODO only check files that actually exists from list: {boot, update, ecoboot}.bin
        if (!checksum_verify(tl, &verify_handle)) {
            trace_write(t, ErrorChecksums, 0);
            goto exit;
        }
    }

    if (handle->enabled.check_version) {
        if (!version_check(tl, &verify_handle)) {
            trace_write(t, ErrorChecksums, 0);
            goto exit;
        }
    }

    if (!tmp_files_move(handle, tl)) {
        trace_write(t, ErrorMove, 0);
        goto exit;
    }

exit:
    return trace_ok(t);
}
