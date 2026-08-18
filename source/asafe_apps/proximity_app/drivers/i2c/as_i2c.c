/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_i2c.c
 * \brief   This file handles all the logic between the I2C Wirepas Driver and the ASAFE Application
 */

#include <stdlib.h>

#include "api.h"
#include "node_configuration.h"
#include "shared_data.h"
#include "app_scheduler.h"

#include "board.h"

#include "i2c.h"

i2c_conf_t i2c_conf;

#define I2C_SPEED 100000 // 100Khz

void asI2CHWClose()
{
    /*Conformant with default config from Nordic*/

    I2C_close();

    // Required due to silicon issue - https://infocenter.nordicsemi.com/index.jsp?topic=%2Ferrata_nRF52832_EngB%2FERR%2FnRF52832%2FEngineeringB%2Flatest%2Fanomaly_832_89.html
    // Without below we get a 400uA static current.
    *(volatile uint32_t *)0x40003FFC = 0;
    *(volatile uint32_t *)0x40003FFC;
    *(volatile uint32_t *)0x40003FFC = 1;

#if defined(BOARD_I2C_PULLUP)
    nrf_gpio_cfg_output(BOARD_I2C_SDA_PIN);
    nrf_gpio_pin_set(BOARD_I2C_SDA_PIN);

    nrf_gpio_cfg_output(BOARD_I2C_SCL_PIN);
    nrf_gpio_pin_set(BOARD_I2C_SCL_PIN);
#else
    nrf_gpio_cfg(BOARD_I2C_SCL_PIN, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(BOARD_I2C_SDA_PIN, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
#endif
}

i2c_res_e asI2CHWOpen()
{
    i2c_conf.clock = I2C_SPEED;
    /*I2C Pullup - To enable define BOARD_I2C_PULLUP*/
#if defined(BOARD_I2C_PULLUP)
    i2c_conf.pullup = true;
#else
    i2c_conf.pullup = false;
#endif

    // Init the I2C Hardware
    return I2C_init(&i2c_conf);
}