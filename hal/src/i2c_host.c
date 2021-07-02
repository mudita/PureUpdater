#include <hal/system.h>
#include <hal/i2c_host.h>
#include <prv/hal/i2c_dev.h>
#include <drivers/fsl_lpi2c.h>

int hal_i2c_init(struct hal_i2c_dev *inst, uint32_t clk_hz)
{
    lpi2c_master_config_t lpi2cConfig = {0};
    LPI2C_MasterGetDefaultConfig(&lpi2cConfig);
    LPI2C_MasterInit((LPI2C_Type *)inst->base, &lpi2cConfig, clk_hz);
    return 0;
}

int hal_i2c_send(struct hal_i2c_dev *inst, uint8_t dev_addr, const uint8_t *dev_subaddr,
                 uint8_t subaddr_size, const void *buf, uint8_t buf_size)
{
    int ret;
    do
    {
        ret = LPI2C_MasterStart((LPI2C_Type *)inst->base, dev_addr, kLPI2C_Write);
        if (ret != kStatus_Success)
            break;
        while (LPI2C_MasterGetStatusFlags((LPI2C_Type *)inst->base) & kLPI2C_MasterNackDetectFlag)
            ;
        ret = LPI2C_MasterSend((LPI2C_Type *)inst->base, dev_subaddr, subaddr_size);
        if (ret != kStatus_Success)
            break;
        ret = LPI2C_MasterSend((LPI2C_Type *)inst->base, buf, buf_size);
        if (ret != kStatus_Success)
            break;
        ret = LPI2C_MasterStop((LPI2C_Type *)inst->base);
        if (ret != kStatus_Success)
            break;
    } while (0);
    return ret;
}

int hal_i2c_receive(struct hal_i2c_dev *inst, uint8_t dev_addr, const uint8_t *dev_subaddr,
                    uint8_t subaddr_size, void *buf, uint8_t buf_size)
{
    int ret;
    do
    {
        ret = LPI2C_MasterStart((LPI2C_Type *)inst->base, dev_addr, kLPI2C_Write);
        if (ret != kStatus_Success)
            break;
        while (LPI2C_MasterGetStatusFlags((LPI2C_Type *)inst->base) & kLPI2C_MasterNackDetectFlag)
            ;
        ret = LPI2C_MasterSend((LPI2C_Type *)inst->base, dev_subaddr, subaddr_size);
        if (ret != kStatus_Success)
            break;
        ret = LPI2C_MasterRepeatedStart((LPI2C_Type *)inst->base, dev_addr, kLPI2C_Read);
        if (ret != kStatus_Success)
            break;
        ret = LPI2C_MasterReceive((LPI2C_Type *)inst->base, buf, buf_size);
        if (ret != kStatus_Success)
            break;
        ret = (LPI2C_MasterStop((LPI2C_Type *)inst->base));
        if (ret != kStatus_Success)
            break;
    } while (0);
    return ret;
}
