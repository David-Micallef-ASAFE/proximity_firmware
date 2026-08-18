/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_lis3dh.c
 * \brief   This file handles all the logic between the lis3dh driver and ASAFE application
 */

#include <stdlib.h>

#include "api.h"
#include "node_configuration.h"
#include "shared_data.h"
#include "app_scheduler.h"

#include "lis3dh.h"
#include "i2c.h"
#include "as_i2c.h"

#define DEBUG_LOG_MODULE_NAME "AS LIS3DH"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

#define I2C_ADDRESS_L LIS3DH_I2C_ADD_L
#define I2C_ADDRESS_H LIS3DH_I2C_ADD_H

i2c_xfer_t i2c_xfer;

stmdev_ctx_t dev_ctx;

static uint8_t i2cTX_buffer[32];

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{
    i2cTX_buffer[0] = reg;
    memcpy(&i2cTX_buffer[1], bufp, len);
    i2c_xfer.address = (I2C_ADDRESS_H >> 1);
    i2c_xfer.write_ptr = i2cTX_buffer;
    i2c_xfer.write_size = 1 + len;
    i2c_xfer.read_size = 0;
    i2c_xfer.read_ptr = NULL;
    I2C_transfer(&i2c_xfer, NULL);
    return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
    reg |= 0x80;
    i2cTX_buffer[0] = reg;

    i2c_xfer.address = (I2C_ADDRESS_H >> 1);
    i2c_xfer.write_ptr = i2cTX_buffer;
    i2c_xfer.write_size = 1;
    i2c_xfer.read_size = 0;
    i2c_xfer.read_ptr = NULL;
    I2C_transfer(&i2c_xfer, NULL);

    i2c_xfer.address = (I2C_ADDRESS_H >> 1);
    i2c_xfer.write_ptr = NULL;
    i2c_xfer.write_size = 0;
    i2c_xfer.read_size = len;
    i2c_xfer.read_ptr = bufp;
    I2C_transfer(&i2c_xfer, NULL);
    return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 */
void asLIS3DHInit()
{
    uint8_t whoamI, dummy;

    asI2CHWOpen();

    /* Initialize mems driver interface */
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;
    dev_ctx.handle = NULL;
    /* Initialize platform specific hardware */

    lis3dh_int1_cfg_t int1_cfg;
    lis3dh_ctrl_reg3_t ctrl_reg3;

    /*Check device ID*/
    lis3dh_device_id_get(&dev_ctx, &whoamI);

    if (whoamI != LIS3DH_ID)
    {
        // lib_state->stopStack();
        LOG(LVL_ERROR, "LIS3DH Error\n");
        return;
    }

    LOG(LVL_INFO, "LIS3DH OK\n");

    /*Put the lis3dh in low power*/
    lis3dh_data_rate_set(&dev_ctx, LIS3DH_POWER_DOWN);

    /*Close I2C*/
    asI2CHWClose();

    return;

    /*Reset the accel*/
    dummy = 0x10;
    lis3dh_write_reg(&dev_ctx, LIS3DH_CTRL_REG0, &dummy, 1);
    dummy = 0x00;
    lis3dh_write_reg(&dev_ctx, LIS3DH_TEMP_CFG_REG, &dummy, 1);
    dummy = 0x07;
    lis3dh_write_reg(&dev_ctx, LIS3DH_CTRL_REG1, &dummy, 1);
    dummy = 0x00;
    lis3dh_write_reg(&dev_ctx, LIS3DH_CTRL_REG2, &dummy, 1);
    dummy = 0x00;
    lis3dh_write_reg(&dev_ctx, LIS3DH_CTRL_REG3, &dummy, 1);
    dummy = 0x00;
    lis3dh_write_reg(&dev_ctx, LIS3DH_CTRL_REG4, &dummy, 1);
    dummy = 0x00;
    lis3dh_write_reg(&dev_ctx, LIS3DH_CTRL_REG5, &dummy, 1);
    dummy = 0x00;
    lis3dh_write_reg(&dev_ctx, LIS3DH_CTRL_REG6, &dummy, 1);

    /* High-pass filter enabled on interrupt activity 1 */
    lis3dh_high_pass_int_conf_set(&dev_ctx, LIS3DH_ON_INT1_GEN);
    /* Enable HP filter for wake-up event detection
     *
     * Use this setting to remove gravity on data output */
    lis3dh_high_pass_on_outputs_set(&dev_ctx, PROPERTY_ENABLE);
    /* Enable AOI1 on int1 pin */
    lis3dh_pin_int1_config_get(&dev_ctx, &ctrl_reg3);
    ctrl_reg3.i1_ia1 = PROPERTY_ENABLE;
    lis3dh_pin_int1_config_set(&dev_ctx, &ctrl_reg3);
    /* Interrupt 1 pin latched */
    lis3dh_int1_pin_notification_mode_set(&dev_ctx,
                                          LIS3DH_INT1_PULSED);
    /* Set full scale to 2 g */
    lis3dh_full_scale_set(&dev_ctx, LIS3DH_2g);
    /* Set interrupt threshold to 0x10 -> 250 mg */
    lis3dh_int1_gen_threshold_set(&dev_ctx, 0x06);
    // lis3dh_int1_gen_threshold_set(&dev_ctx, 0x0A);
    /* Set no time duration */
    lis3dh_int1_gen_duration_set(&dev_ctx, 0);
    /* Dummy read to force the HP filter to current acceleration value. */
    lis3dh_filter_reference_get(&dev_ctx, &dummy);
    /* Configure wake-up interrupt event on all axis */
    lis3dh_int1_gen_conf_get(&dev_ctx, &int1_cfg);
    int1_cfg.zhie = PROPERTY_ENABLE;
    int1_cfg.yhie = PROPERTY_ENABLE;
    int1_cfg.xhie = PROPERTY_ENABLE;
    int1_cfg.aoi = PROPERTY_DISABLE;
    lis3dh_int1_gen_conf_set(&dev_ctx, &int1_cfg);

    /*Initialize the Interrupt Logic*/

    asI2CHWClose();
}