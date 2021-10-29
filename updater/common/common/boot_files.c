#include "boot_files.h"

const size_t backup_boot_files_list_size = 3;
const char *backup_boot_files[] =
        {
                "updater.bin",
                "boot.bin",
                "version.json",
        };

const size_t verify_files_list_size = 3;
const char *verify_files[] =
        {
                "updater.bin",
                "boot.bin",
                "ecoboot.bin",
        };

const size_t db_extensions_list_size = 4;
const char *db_extensions[] =
        {
                ".db",
                ".db-journal",
                ".db-wal",
                ".directory_is_indexed",
        };
