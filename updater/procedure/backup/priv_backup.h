#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "backup.h"
#include <common/log.h>

/// assert that paths from -> to are proper
bool check_backup_entries(const struct backup_handle_s *handle);

/// backup only required data stored on 1:/ (boot) partition
/// - updater.bin
/// - boot.bin
/// - version.json
bool backup_boot_files(const struct backup_handle_s *handle);

/// backup only required data stored on 3:/ (user) partition
/// in 'db' folder
bool backup_databases(const struct backup_handle_s *handle);

/// backup only required data stored on 3:/ (user) partition
/// all *.log and *.json files
bool backup_logs(const struct backup_handle_s *handle);

/// UNUSED:

/// backup whole directory recursively
bool backup_whole_directory(const struct backup_handle_s *handle);

#ifdef __cplusplus
}
#endif
