#pragma once

#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct version_json_file_s {
    char *name;
    char *md5sum;
    char *version;
    bool valid;
} version_json_file_s;

typedef struct version_json_s {
    version_json_file_s boot;
    version_json_file_s bootloader;
    version_json_file_s updater;
    bool valid;
} version_json_s;

typedef struct verify_file_handle_s {
    const char *file_to_verify;
    version_json_s current_version_json;
    version_json_s version_json;
} verify_file_handle_s;

typedef struct version_s {
    int major;
    int minor;
    int patch;
    char *str;
    bool valid;
} version_s;

#ifdef __cplusplus
}
#endif
