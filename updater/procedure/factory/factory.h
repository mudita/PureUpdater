#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <common/trace.h>

enum factory_error_e {
    FactoryOk,
    FactoryErrorTempWlk,
};

struct factory_reset_handle{
    const char * user_dir;
};

bool factory_reset(const struct factory_reset_handle *handle, trace_list_t *tl);

#ifdef __cplusplus
}
#endif
