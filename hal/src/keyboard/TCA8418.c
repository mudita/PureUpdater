/**
 * @file TCA8418.c
 * @brief Basic IO driver for TCA8418 keyboard controller
 * @copyright Copyright (C) 2021 mudita.com.
 */

#include <hal/i2c_host.h>
#include "TCA8418.h"

int TCA8418_SendByte(struct hal_i2c_dev* inst, uint8_t subAddress, uint8_t byte)
{
	return hal_i2c_send(inst, TCA8418_I2C_ADDRESS, &subAddress, sizeof(uint8_t),
        &byte ,sizeof(uint8_t));
}

int TCA8418_ReceiveByte(struct hal_i2c_dev* inst, uint8_t subAddress, uint8_t *byte)
{
	return hal_i2c_receive(inst, TCA8418_I2C_ADDRESS, &subAddress, sizeof(uint8_t),
        byte, sizeof(uint8_t));
}

int TCA8418_ModifyReg(struct hal_i2c_dev* inst, uint8_t subAddress,
    uint8_t mask, bool setClr)
{
	status_t ret = kStatus_Success;

	uint8_t rx = 0;
	ret |= hal_i2c_receive(inst, TCA8418_I2C_ADDRESS, &subAddress, sizeof(uint8_t),
        &rx, sizeof(uint8_t));

	// Set specified bits
	if (setClr){
		rx |= mask;
	} else {
		rx &= ~mask;
	}

    ret |= hal_i2c_receive(inst, TCA8418_I2C_ADDRESS, &subAddress, sizeof(uint8_t),
        &rx, sizeof(uint8_t));

    return ret;
}
