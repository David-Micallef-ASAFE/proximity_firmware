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

#include "gpio.h"
#include "board.h"

#include "as_proximity.h"
#include "as_led.h"

#define DEBUG_LOG_MODULE_NAME "AS LIS3DH"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

#define I2C_ADDRESS_L LIS3DH_I2C_ADD_L
#define I2C_ADDRESS_H LIS3DH_I2C_ADD_H

/*Device ID retries cover the 5ms LIS3DH boot time after power up*/
#define LIS3DH_ID_RETRIES 10
#define LIS3DH_ID_RETRY_DELAY_US 1000

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
    return (int32_t)I2C_transfer(&i2c_xfer, NULL);
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

    /*Sub address write followed by a repeated start read in a single transfer, as per the LIS3DH datasheet*/
    i2c_xfer.address = (I2C_ADDRESS_H >> 1);
    i2c_xfer.write_ptr = i2cTX_buffer;
    i2c_xfer.write_size = 1;
    i2c_xfer.read_size = len;
    i2c_xfer.read_ptr = bufp;
    return (int32_t)I2C_transfer(&i2c_xfer, NULL);
}

/*
 * @brief  Busy wait, used while the LIS3DH completes its boot sequence
 */
static void lis3dh_delay_us(uint32_t us)
{
    app_lib_time_timestamp_hp_t end = lib_time->addUsToHpTimestamp(lib_time->getTimestampHp(), us);

    while (lib_time->isHpTimestampBefore(lib_time->getTimestampHp(), end))
        ;
}

static bool lis3dhPresent = false;

/*
 * @brief  Callback function for the interrupt pin.  Called on the rising edge of LIS3DH INT1 (motion detected).
 */
static void lis3dh_event_cb(gpio_id_t id, gpio_in_event_e event)
{
    (void)id;
    (void)event;
    LOG(LVL_INFO, "Motion Detected");
#ifdef AS_RTLS_ROLE_TAG
    asProximityMotionDetected();
#endif
}

bool asLIS3DHInMotion()
{
    return (bool)nrf_gpio_pin_read(LIS3DH_INTERRUPT_1_PIN);
}

/*
 * @brief  Initialize the LIS3DH: verify the device ID and leave it powered down with no interrupts enabled.
 *         Call asLIS3DHEnableMotionDetect() to start motion detection.
 */
void asLIS3DHInit()
{
    uint8_t whoamI;
    uint8_t dummy;

/*Initialize the SDO pin if its defined.  Note the SDO is internally pulled up in the accel so putting it to gnd will waste energy*/
#ifdef LIS3DH_SDO
    nrf_gpio_cfg_output(LIS3DH_SDO);
    nrf_gpio_pin_set(LIS3DH_SDO);
#endif

    lis3dhPresent = false;

    asI2CHWOpen();

    /* Initialize mems driver interface */
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;
    dev_ctx.handle = NULL;

    /*Check device ID.  After power up the LIS3DH needs up to 5ms to boot and will not respond until then, so retry*/
    int32_t res = 0;
    whoamI = 0;
    for (uint8_t attempt = 0; attempt < LIS3DH_ID_RETRIES; attempt++)
    {
        res = lis3dh_device_id_get(&dev_ctx, &whoamI);
        if ((res == I2C_RES_OK) && (whoamI == LIS3DH_ID))
        {
            break;
        }
        lis3dh_delay_us(LIS3DH_ID_RETRY_DELAY_US);
    }

    if ((res != I2C_RES_OK) || (whoamI != LIS3DH_ID))
    {
        LOG(LVL_ERROR, "LIS3DH Error i2c res %d whoami 0x%02x\n", res, whoamI);
        asI2CHWClose();
        return;
    }

    lis3dhPresent = true;

    /*The accel keeps its registers across an MCU-only reset, so explicitly disable the interrupt routing and generator*/
    dummy = 0x00;
    lis3dh_write_reg(&dev_ctx, LIS3DH_CTRL_REG3, &dummy, 1);
    dummy = 0x00;
    lis3dh_write_reg(&dev_ctx, LIS3DH_INT1_CFG, &dummy, 1);

    /*Power down*/
    lis3dh_data_rate_set(&dev_ctx, LIS3DH_POWER_DOWN);

    asI2CHWClose();

    LOG(LVL_INFO, "LIS3DH OK, powered down\n");
}

/*
 * @brief  Enable the LIS3DH in 10Hz low power motion detection and register the INT1 interrupt.
 *         asLIS3DHInit() must have been called first.
 *
 * @return true if motion detection was enabled
 */
bool asLIS3DHEnableMotionDetect()
{
    uint8_t dummy;
    lis3dh_int1_cfg_t int1_cfg;
    lis3dh_ctrl_reg3_t ctrl_reg3;

    if (!lis3dhPresent)
    {
        LOG(LVL_ERROR, "LIS3DH not present, cannot enable motion detect");
        return false;
    }

    asI2CHWOpen();

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
    /* Interrupt 1 pin pulsed */
    lis3dh_int1_pin_notification_mode_set(&dev_ctx,
                                          LIS3DH_INT1_PULSED);
    /* Set full scale to 2 g */
    lis3dh_full_scale_set(&dev_ctx, LIS3DH_2g);
    /* Set interrupt threshold to 0x06 -> ~96 mg */
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

    /*Put the lis3dh in low power*/
    lis3dh_data_rate_set(&dev_ctx, LIS3DH_ODR_10Hz);

    asI2CHWClose();

    /*Initialize the Interrupt Logic*/
    gpio_res_e gpio_res;
    const gpio_in_cfg_t motion_mon_int_cfg =
        {
            .event_cb = lis3dh_event_cb,
            .event_cfg = GPIO_IN_EVENT_RISING_EDGE,
            .in_mode_cfg = GPIO_IN_PULL_NONE};

    gpio_res = Gpio_inputSetCfg(BOARD_GPIO_ID_LIS3DH, &motion_mon_int_cfg);

    if (gpio_res != GPIO_RES_OK)
    {
        LOG(LVL_ERROR, "Cannot enable interrupt pin %d", gpio_res);
        return false;
    }

    LOG(LVL_INFO, "LIS3DH motion detect enabled, INT pin registered");
    return true;
}
