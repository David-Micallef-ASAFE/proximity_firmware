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
#ifndef BOARD_C3W_BOARD_H_
#define BOARD_C3W_BOARD_H_

// Serial port pins
#define BOARD_USART_TX_PIN 4 // Connected to LED1 and labeled as RXD1 test point
#define BOARD_USART_RX_PIN 8
#define BOARD_USART_CTS_PIN 7 /* For USE_USART_HW_FLOW_CONTROL */
#define BOARD_USART_RTS_PIN 5 /* For USE_USART_HW_FLOW_CONTROL */

// LED Definitions
//  Active low polarity for LEDs
#define BOARD_LED_ACTIVE_LOW false

// List of GPIO pins for the LEDs on the board.  This is based on the amount of LEDs on the board
// MBT02 only has 1 LED
#define BOARD_LED_PIN_LIST \
    {                      \
        10}

// BUZZER
#define BUZZER_PIN 2
// The EN1 and EN2 pins are used on the MBT02 by minew for the piezo driver
#define BUZZER_EN1_PIN 4
#define BUZZER_EN2_PIN 3

// I2C settings
#define USE_I2C1
#define BOARD_I2C_SDA_PIN 20
#define BOARD_I2C_SCL_PIN 9

// LIS3DH
#define LIS3DH_INTERRUPT_1_PIN 7
#define LIS3DH_INTERRUPT_2_PIN 8
#define LIS3DH_SDO 17

// SPI
#define USE_SPI0
#define BOARD_SPI_SCK_PIN 31
#define BOARD_SPI_MOSI_PIN 30
#define BOARD_SPI_MISO_PIN 28

// DW1000
#define DW_CS_PIN 2
#define DW_IRQ_PIN 3
// #define DW1000_WAKEUP_PIN 20 // Active high
#define DW_RST_PIN 29 // Active Low

// List of GPIO pins
#define BOARD_GPIO_PIN_LIST \
    {                       \
        4,                  \
        3,                  \
        NRF_GPIO_PIN_MAP(1, 9)}

// User friendly name for GPIOs (IDs mapped to the BOARD_GPIO_PIN_LIST table)
#define BOARD_GPIO_ID_BUTTON0 0 // mapped to pin P0.13
#define BOARD_GPIO_ID_DW_IRQ 1  // mapped to pin P0.3
#define BOARD_GPIO_ID_LIS3DH 2  // mapped to pin P1.09

// List of button IDs
#define BOARD_BUTTON_ID_LIST {BOARD_GPIO_ID_BUTTON0}

// Active low polarity for buttons
#define BOARD_BUTTON_ACTIVE_LOW true

// Active internal pull-up for buttons
#define BOARD_BUTTON_INTERNAL_PULL true

// Battery measured through a divider (VDD is behind an LDO). See as_battery.h
#define BOARD_VBAT_PIN 5 // AIN3
#define BOARD_VBAT_R_TOP_KOHM 1000    // VBAT -> AIN3
#define BOARD_VBAT_R_BOTTOM_KOHM 1000 // AIN3 -> GND

// The board supports DCDC (#define BOARD_SUPPORT_DCDC)
// Since SDK v1.2 (bootloader > v7) this option has been move to
// board/<board_name>/config.mk. Set board_hw_dcdc to yes to enable DCDC.
#ifdef BOARD_SUPPORT_DCDC
#error This option has been moved to board/<board_name>/config.mk
#endif

#endif /* BOARD_C3W_BOARD_H_ */
