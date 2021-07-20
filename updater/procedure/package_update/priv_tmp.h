#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "update.h"
#include "common/trace.h"

/// create temporary catalog for update - removes old catalog if exists in that place
bool tmp_create_catalog(struct update_handle_s *handle, trace_list_t *tl);
/// Move files:
/// 1. recursive: from {catalog}/{handle_os} move to {handle os}
/// 2. recursive: from {catalog}/{handle_user} move to {handle user}
bool tmp_files_move(struct update_handle_s *handle, trace_list_t *tl);

#ifdef __cplusplus
}
#endif
