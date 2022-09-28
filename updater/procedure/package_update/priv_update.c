#include <common/tar.h>
#include <common/match.h>
#include <microtar/microtar.h>
#include <string.h>
#include "priv_update.h"
#include "procedure/checksum/checksum.h"

bool is_os_file(const char *file) {
    const char *os_files[] = {
            "boot.bin",
            "ecoboot.bin",
            "updater.bin",
            "version.json",
            ".boot.json",
            ".boot.json.crc",
            "country-codes.db", /// WARN: this is bad, but this is how our MuditaOS works
            "Luts.bin",
            "checksums.txt"
    };

    return string_match_any_of(file, os_files, sizeof(os_files) / sizeof(os_files[0]));
}

/// dumb put to fat partition
bool should_not_be_on_os_but_is(const char *file) {
    const char *os_files[] = {
            "assets",
    };

    return string_match_any_of_partial(file, os_files, sizeof(os_files) / sizeof(os_files[0]));
}

bool unpack(struct update_handle_s *handle) {
    bool ret = true;
    int result = 0;
    struct tar_ctx ctx;

    do {
        if (0 != tar_init(&ctx, handle->update_from, "r")) {
            debug_log("Update: unable to init tar archive: %s", handle->update_from);
            ret = false;
            break;
        }

        int lib_error = MTAR_ESUCCESS;
        mtar_header_t header;

        while ((lib_error = (mtar_read_header(&ctx.tar, &header))) != MTAR_ENULLRECORD) {
            lib_error = mtar_read_header(&ctx.tar, &header);

            if (lib_error != 0 && lib_error != MTAR_ENULLRECORD) {
                break;
            }

            const char *to = handle->tmp_user;
            if (is_os_file(header.name) || should_not_be_on_os_but_is(header.name)) {
                to = handle->tmp_os;
            }

            if (header.type == MTAR_TDIR) {
                result = un_tar_catalog(&ctx, &header, to);
            } else if (header.type == MTAR_TREG) {
                result = un_tar_file(&ctx, &header, to);
            }

            if (result != 0) {
                ret = false;
                break;
            }

            if (tar_next(&ctx)) {
                ret = false;
                break;
            }
        }

        if (0 != lib_error && lib_error != MTAR_ENULLRECORD) {
            debug_log("Update: tar error %d", lib_error);
            ret = false;
            break;
        }
    } while (0);


    if (tar_deinit(&ctx)) {
        ret = false;
    }
    return ret;
}
