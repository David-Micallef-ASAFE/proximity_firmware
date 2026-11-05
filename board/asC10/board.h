/* Copyright 2017 Wirepas Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * @file
 *
 * Board definition for the
 * <a href="https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52-DK">Nordic semiconductor PCA10040 evaluation board</a>
 */
#ifndef BOARD_C10_BOARD_H_
#define BOARD_C10_BOARD_H_

// Serial port pins
#define BOARD_USART_TX_PIN 5 // Not coming out to a test point
#define BOARD_USART_RX_PIN 8
#define BOARD_USART_CTS_PIN 7 /* For USE_USART_HW_FLOW_CONTROL */
#define BOARD_USART_RTS_PIN 5 /* For USE_USART_HW_FLOW_CONTROL */

// List of GPIO pins
#define BOARD_GPIO_PIN_LIST { \
    13, 7}

// User friendly name for GPIOs (IDs mapped to the BOARD_GPIO_PIN_LIST table)
#define BOARD_GPIO_ID_BUTTON0 0 // mapped to pin P0.13
#define BOARD_GPIO_ID_LIS3DH 1  // mapped to pin P0.7

// List of button IDs
#define BOARD_BUTTON_ID_LIST {BOARD_GPIO_ID_BUTTON0}

// Active low polarity for buttons
#define BOARD_BUTTON_ACTIVE_LOW true

// Active internal pull-up for buttons
#define BOARD_BUTTON_INTERNAL_PULL true

// LED Definitions
//  Active low polarity for LEDs
#define BOARD_LED_ACTIVE_LOW false

// List of GPIO pins for the LEDs on the board.  This is based on the amount of LEDs on the board
// MBT02 only has 1 LED
#define BOARD_LED_PIN_LIST \
    {                      \
        17, 18}

// I2C settings
#define USE_I2C0
#define BOARD_I2C_SDA_PIN 12
#define BOARD_I2C_SCL_PIN 11

// LIS3DH
#define LIS3DH_INTERRUPT_1_PIN 7
#define LIS3DH_INTERRUPT_2_PIN 8

// The board supports DCDC (#define BOARD_SUPPORT_DCDC)
// Since SDK v1.2 (bootloader > v7) this option has been move to
// board/<board_name>/config.mk. Set board_hw_dcdc to yes to enable DCDC.
#ifdef BOARD_SUPPORT_DCDC
#error This option has been moved to board/<board_name>/config.mk
#endif

#endif /* BOARD_MBT02_BOARD_H_ */
