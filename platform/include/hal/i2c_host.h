#pragma once

#include <stdint.h>

//! Device structre
struct hal_i2c_dev;

//Typedef
typedef struct hal_i2c_dev hal_i2c_dev_t;

/** Initialize the I2c controller 
 * @param inst i2c device instance
 * @param clk_hz Clock peripheral frequency
 * @return Error code
 */
int hal_i2c_init(struct hal_i2c_dev* inst, uint32_t clk_hz);

/** Send message via i2c bus
 * @param inst I2c device instance
 * @param dev_addr Device hardware address
 * @param dev_subaddres Device subaddress
 * @param dev_subaddr_size Device subaddress size
 * @param buf Data buffer to send
 * @param buf_size Buffer size
 * @return 0 otherwise error
 */
int hal_i2c_send(struct hal_i2c_dev *inst, uint8_t dev_addr, const uint8_t *dev_subaddr,
    uint8_t subaddr_size, const void *buf, uint8_t buf_size);

 /** Receove message via i2c bus
 * @param inst I2c device instance
 * @param dev_addr Device hardware address
 * @param dev_subaddres Device subaddress
 * @param dev_subaddr_size Device subaddress size
 * @param buf Data buffer to send
 * @param buf_size Buffer size
 * @return 0 otherwise error
 */
int hal_i2c_receive(struct hal_i2c_dev *inst, uint8_t dev_addr, const uint8_t *dev_subaddr,
    uint8_t subaddr_size, void *buf, uint8_t buf_size);
