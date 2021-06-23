#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <common/trace.h>

    enum update_error_e
    {
        ErrorUpdateOk = 0,
        ErrorUnpack,
        ErrorFS,
    };

    struct update_handle_s
    {
        const char *update_from;           /// location we want get tar from
        const char *update_unpack;         /// location we want to unpack the update os: boot.bin, version.json ecoboot.bin
                                           /// on target this would be partition MUDITAOS
        const char *update_os    ;         /// location we want to move OS data
        const char *update_user;           /// location we want to update the update user data: assets, sql etc
                                           /// on target this would mean partition nr 3
        const char *previous_version_json; /// previous update version.json file
    };

    bool update_firmware(struct update_handle_s *handle, trace_list_t *tl);
    const char *update_strerror(int e);

#ifdef __cplusplus
}
#endif
