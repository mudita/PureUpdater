#pragma once

/** System initialize setup */
void sysinit_setup(void);

struct hal_i2c_dev;

/** Get I2C bus controller */
struct hal_i2c_dev* get_i2c_controller();