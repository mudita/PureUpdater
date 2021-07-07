#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <common/enum_s.h>
#include "backup.h"
#include "dir_walker.h"
#include "priv_backup.h"

bool backup_previous_firmware(struct backup_handle_s *handle, trace_list_t *tl)
{
    if (handle == NULL || tl == NULL) {
        perror("No handle or trace");
        return false;
    }

    if (!check_backup_entries(handle, tl)) {
        return false;
    }

    if (!backup_boot_partition(handle, tl)) {
        return false;
    }

    if (!backup_user_data(handle, tl)) {
        return false;
    }

    return true;
}

const char *backup_strerror(int err)
{
    switch(err){
        ENUMS(BackupSuccess);
        ENUMS(BackupErrorAny);
        ENUMS(BackupErrorTar);
        ENUMS(BackupBadInput);
    }
    return "UNDEFINED";
}
