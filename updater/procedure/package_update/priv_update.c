#include <common/tar.h>
#include <common/match.h>
#include <common/trace.h>
#include <microtar/src/microtar.h>
#include <string.h>
#include "priv_update.h"

bool is_os_file(const char *file)
{
    const char *os_files[] = {
        "boot.bin",
        "ecoboot.bin",
        "version.json",
    };

    return string_match_any_of(file, os_files, sizeof(os_files) / sizeof(os_files[0]));
}

static const char *unpack_strerror_ext(int err, int ext_err)
{
    return mtar_strerror(ext_err);
}

bool unpack(struct update_handle_s *handle, trace_list_t *trace_list)
{
    bool ret       = true;
    int result     = 0;
    trace_t *t     = trace_append("unpack", trace_list, update_strerror, unpack_strerror_ext);
    trace_t *tar_t = trace_append("tar", trace_list, tar_strerror, tar_strerror_ext);
    struct tar_ctx ctx;

    do {
        if (0 != tar_init(&ctx, tar_t, handle->update_from, "r")) {
            trace_write(t, ErrorUnpack, 0);
            ret = false;
            break;
        }

        int lib_error = MTAR_ESUCCESS;
        mtar_header_t header;
        while ((lib_error = (mtar_read_header(&ctx.tar, &header))) && MTAR_ENULLRECORD != lib_error ||
               MTAR_ENULLRECORD != lib_error) {

            if (0 != lib_error && lib_error != MTAR_ENULLRECORD) {
                trace_write(t, ErrorUnpack, 0);
                ret = false;
                break;
            }

            const char *to = handle->update_user;
            if (is_os_file(header.name)) {
                to = handle->update_os;
            }

            if (header.type == MTAR_TDIR) {
                result = un_tar_catalog(&ctx, &header, to);
            }
            else if (header.type == MTAR_TREG) {
                result = un_tar_file(&ctx, &header, to);
            }
            else {
                trace_write(t, ErrorUnpack, 1);
            }

            if (result != 0) {
                trace_write(t, ErrorUnpack, 0);
                ret = false;
                break;
            }

            if ( tar_next(&ctx))
            {
                trace_write(t, ErrorUnpack,0);
                ret = false;
                break;
            }

        }
    } while (0);

    if (tar_deinit(&ctx)) {
        trace_write(t, ErrorUnpack, 0);
        ret = false;
    }
    return ret;
}
