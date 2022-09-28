#include "backup.h"
#include "priv_backup.h"
#include <unistd.h>

bool backup_user_databases(const struct backup_handle_s *handle) {
    if (handle == NULL) {
        debug_log("Backup: no handle");
        return false;
    }

    if (!check_backup_entries(handle)) {
        return false;
    }

    unlink(handle->backup_to);

    if (!backup_databases(handle)) {
        return false;
    }

    return true;
}

bool backup_previous_firmware(const struct backup_handle_s *handle) {
    if (handle == NULL) {
        debug_log("Backup: no handle");
        return false;
    }

    if (!check_backup_entries(handle)) {
        return false;
    }

    unlink(handle->backup_to);

    if (!backup_boot_files(handle)) {
        return false;
    }

    if (!backup_user_data(handle)) {
        return false;
    }

    return true;
}
