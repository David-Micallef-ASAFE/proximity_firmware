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
#ifndef BOARD_MBT02_BOARD_H_
#define BOARD_MBT02_BOARD_H_

// Serial port pins
#define BOARD_USART_TX_PIN 17 // labeled as DTM_RX on the schematic
#define BOARD_USART_RX_PIN 8
#define BOARD_USART_CTS_PIN 27 /* For USE_USART_HW_FLOW_CONTROL */
#define BOARD_USART_RTS_PIN 5  /* For USE_USART_HW_FLOW_CONTROL */

// List of GPIO pins - This is used
#define BOARD_GPIO_PIN_LIST {13, 7}

// User friendly name for GPIOs (IDs mapped to the BOARD_GPIO_PIN_LIST table)
#define BOARD_GPIO_ID_BUTTON0 0 // mapped to pin P0.13 - Button
#define BOARD_GPIO_ID_LIS3DH 1  // mapped to pin P0.07 - LIS3DH INT1

// List of button IDs
#define BOARD_BUTTON_ID_LIST {BOARD_GPIO_ID_BUTTON0}

// Active low polarity for buttons
#define BOARD_BUTTON_ACTIVE_LOW true

// Active internal pull-up for buttons
#define BOARD_BUTTON_INTERNAL_PULL false

// LED Definitions
//  Active low polarity for LEDs
#define BOARD_LED_ACTIVE_LOW false

// List of GPIO pins for the LEDs on the board.  This is based on the amount of LEDs on the board
// MBT02 only has 1 LED
#define BOARD_LED_PIN_LIST \
    {                      \
        18}

// BUZZER
#define BUZZER_PIN 2
// The EN1 and EN2 pins are used on the MBT02 by minew for the piezo driver
#define BUZZER_EN1_PIN 4
#define BUZZER_EN2_PIN 3

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
