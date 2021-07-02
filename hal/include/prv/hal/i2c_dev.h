#pragma once
#include <stdint.h>
#include <stdbool.h>

struct hal_i2c_dev
{
    uintptr_t base;
    bool initialized;
    int error;
};
