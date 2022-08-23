#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "backup.h"
#include <common/log.h>

/// assert that paths from -> to are proper
bool check_backup_entries(struct backup_handle_s *handle);
/// backup only required data stored on 1:/ (boot) partition
/// - boot.bin
/// - version.json
bool backup_boot_partition(struct backup_handle_s *handle);
/// backup only required data stored on 3:/ (user) partition
/// all: *db files
bool backup_user_data(struct backup_handle_s *handle);

/// UNUSED:

/// backup whole directory recursively
bool backup_whole_directory(struct backup_handle_s *handle);

#ifdef __cplusplus
}
#endif
