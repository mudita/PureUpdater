#pragma once

/** System initialize setup */
void system_initialize(void);

struct hal_i2c_dev;

/** Get I2C bus controller */
struct hal_i2c_dev *get_i2c_controller();
