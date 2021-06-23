#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "update.h"
#include "common/trace.h"

bool unpack(struct update_handle_s *handle, trace_list_t *);

#ifdef __cplusplus
}
#endif
