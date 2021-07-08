#pragma once

/** System initialize setup */
void system_initialize(void);

struct hal_i2c_dev;

/** Get I2C bus controller */
struct hal_i2c_dev *get_i2c_controller();

//! Boot reason code
enum system_boot_reason_code
{
    system_boot_reason_update,   //! Restart caused by the update request
    system_boot_reason_recovery, //! Restart caused by the recovery request
    system_boot_reason_factory,  //! Restart caused by the factory reset request
    system_boot_reason_unknown,  //! Unknown boot reason code
};

/** Get the system boot resason code
 * @return Boot reason code
 */
enum system_boot_reason_code system_boot_reason(void);

/** Get the system boot reason code string
 * @return Boot reason string
 */
const char *system_boot_reason_str(enum system_boot_reason_code code);
