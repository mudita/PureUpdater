#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <common/log.h>

enum backup_error_e {
    BackupSuccess,
    BackupErrorAny,
    BackupErrorTar,
    BackupBadInput,
};

/// all input data required for backup
struct backup_handle_s {
    const char *backup_from_os;   /// os location we want to tar
    const char *backup_from_user; /// user location we want to tar
    const char *backup_to;        /// tar file to put backup in
};

bool backup_user_databases(const struct backup_handle_s *handle);
bool backup_previous_firmware(const struct backup_handle_s *handle);

#ifdef __cplusplus
}
#endif
