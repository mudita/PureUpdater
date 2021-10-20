#pragma once

/** System initialize setup */
void system_initialize(void);

/** Deinitialize the system library */
void system_deinitialize(void);

struct hal_i2c_dev;

/** Get I2C bus controller */
struct hal_i2c_dev *get_i2c_controller();
