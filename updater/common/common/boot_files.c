#include "boot_files.h"

const uint16_t backup_boot_files_list_size = 3;
const char *backup_boot_files_list[] =
        {
                "updater.bin",
                "boot.bin",
                "version.json",
        };

const uint16_t verify_files_list_size = 3;
const char *verify_files_list[] =
        {
                "updater.bin",
                "boot.bin",
                "ecoboot.bin",
        };
