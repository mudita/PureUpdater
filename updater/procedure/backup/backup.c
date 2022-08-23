#include <errno.h>
#include "backup.h"
#include "dir_walker.h"
#include "priv_backup.h"

bool backup_previous_firmware(struct backup_handle_s *handle) {
    if (handle == NULL) {
        debug_log("Backup: no handle");
        return false;
    }

    if (!check_backup_entries(handle)) {
        return false;
    }

    if (!backup_boot_partition(handle)) {
        return false;
    }

    if (!backup_user_data(handle)) {
        return false;
    }

    return true;
}
