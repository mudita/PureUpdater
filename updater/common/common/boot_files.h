#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>

extern const size_t boot_files_to_backup_list_size;
extern const char *boot_files_to_backup[];

extern const size_t files_to_verify_list_size;
extern const char *files_to_verify[];

extern const size_t db_extensions_list_size;
extern const char *db_extensions[];

#ifdef __cplusplus
}
#endif
