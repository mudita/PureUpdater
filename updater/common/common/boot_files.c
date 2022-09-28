#include "boot_files.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

const char *boot_files_to_backup[] =
{
    "updater.bin",
    "boot.bin",
    "version.json",
};
const size_t boot_files_to_backup_list_size = ARRAY_SIZE(boot_files_to_backup);

const char *files_to_verify[] =
{
    "updater.bin",
    "boot.bin",
    "ecoboot.bin",
};
const size_t files_to_verify_list_size = ARRAY_SIZE(files_to_verify);

const char *db_extensions[] =
{
    ".db",
    ".db-journal",
    ".db-wal",
    ".directory_is_indexed",
};
const size_t db_extensions_list_size = ARRAY_SIZE(db_extensions);
