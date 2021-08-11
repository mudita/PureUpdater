#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <common/trace.h>

    enum update_error_e
    {
        ErrorUpdateOk,
        ErrorSignCheck,
        ErrorUnpack,
        ErrorChecksums,
        ErrorTmp,
        ErrorBackup,
        ErrorFactory,
        ErrorVersion,
        ErrorMove,
        ErrorUpdateEcoboot
    };

    struct update_handle_s
    {
        const char *update_from;           /// location we want get tar from
        const char *update_os;             /// location we want to move OS data
        const char *update_user;           /// location we want to update the update user data: assets, sql etc
                                           /// on target this would mean partition nr 3
        const char *previous_version_json; /// previous update version.json file
        const char *backup_full_path;      /// full path where to put backup
        const char *factory_full_path;     /// full path where from to take factory img
        const char *tmp_os;                /// temporary os catalog to perform unpack - to not mv between fs-es
        const char *tmp_user;              /// temporary user catalog to perform unpack - to not mv between fs-es

        /// options to perform with update_firmware
        struct
        {
            bool check_sign : 1;
            bool backup : 1;
            bool check_checksum : 1;
            bool check_version : 1;
        } enabled;
    };

    void update_firmware_init(struct update_handle_s *h);
    bool update_firmware(struct update_handle_s *handle, trace_list_t *tl);
    const char *update_strerror(int e);

#ifdef __cplusplus
}
#endif
