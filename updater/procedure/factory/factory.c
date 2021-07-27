#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <procedure/package_update/priv_tmp.h>
#include "factory.h"

bool factory_reset(const struct factory_reset_handle *handle, trace_list_t *tl) {
    bool success = false;

    if (tl == NULL || handle == NULL) {
        printf("factory_reset trace/handle null error");
        goto exit;
    }
    trace_t *trace = trace_append(__PRETTY_FUNCTION__, tl, NULL, NULL);

    struct stat data;
    int ret = stat(handle->user_dir, &data);

    if (ret == 0 && !recursive_unlink(handle->user_dir, true, trace)) {
        trace_write(trace, FactoryErrorTempWlk, errno);
        goto exit;
    }

    success = true;

    exit:
    return success;
}
