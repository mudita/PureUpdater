#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <common/trace.h>

    enum backup_error_e
    {
        BackupSuccess,
        BackupErrorAny,
        BackupErrorTar,
        BackupBadInput,
    };

    /// all input data required for backup
    struct backup_handle_s
    {
        const char *backup_from; /// location we want to tar
        const char *backup_to;   /// tar file to put backup in
    };

    bool backup_previous_firmware(struct backup_handle_s *handle, trace_list_t *tl);
    const char *backup_strerror(int err);

#ifdef __cplusplus
}
#endif
