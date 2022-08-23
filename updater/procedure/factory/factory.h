#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <common/log.h>

struct factory_reset_handle {
    const char *user_dir;
};

bool factory_reset(const struct factory_reset_handle *handle);

#ifdef __cplusplus
}
#endif
