#pragma once

enum eco_boot_reason_code
{
   eco_update_code = 0xBADC0000,
   eco_recovery_code = 0xBADC0001,
   eco_factory_rst_code = 0xBADC0002,
   eco_factory_pgm_keys_code = 0xBADC0003,
   eco_backup_code = 0xBADC0004
};

//! Boot reason code
enum system_boot_reason_code
{
    system_boot_reason_update,   //! Restart caused by the update request
    system_boot_reason_recovery, //! Restart caused by the recovery request
    system_boot_reason_factory,  //! Restart caused by the factory reset request
    system_boot_reason_pgm_keys, //! Load keys request (close configuration)
    system_boot_reason_backup,   //! Restart caused by the backup request
    system_boot_reason_unknown   //! Unknown boot reason code
};

/** Get the system boot reason code
 * @return Boot reason code
 */
enum system_boot_reason_code system_boot_reason(void);

/** Get the system boot reason code string
 * @return Boot reason string
 */
const char *system_boot_reason_str(enum system_boot_reason_code code);
