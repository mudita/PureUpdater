#include <errno.h>
#include <sys/stat.h>
#include <procedure/package_update/priv_tmp.h>
#include "factory.h"

bool factory_reset(const struct factory_reset_handle *handle) {
    bool success = false;

    if (handle == NULL) {
        debug_log("Factory reset: handle is a NULL");
        goto exit;
    }

    struct stat data;
    int ret = stat(handle->user_dir, &data);
    if (ret != 0) {
        debug_log("Factory reset: failed to stat user dir:%s %d", handle->user_dir, ret);
        goto exit;
    }

    if (!recursive_unlink(handle->user_dir, true)) {
        debug_log("Factory reset: failed to unlink user dir, errno: %d", errno);
        goto exit;
    }

    success = true;

    exit:
    return success;
}
