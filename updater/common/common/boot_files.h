#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>

extern const size_t backup_boot_files_list_size;
extern const char *backup_boot_files[];

extern const size_t verify_files_list_size;
extern const char *verify_files[];

extern const size_t db_extensions_list_size;
extern const char *db_extensions[];

#ifdef __cplusplus
}
#endif
