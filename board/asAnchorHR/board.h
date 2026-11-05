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
#ifndef BOARD_ASANCHORHR_BOARD_H_
#define BOARD_ASANCHORHR_BOARD_H_

// Serial port pins
#define BOARD_USART_TX_PIN 28 // Connected to LED1 and labeled as RXD1 test point
#define BOARD_USART_RX_PIN 11
#define BOARD_USART_CTS_PIN 7 /* For USE_USART_HW_FLOW_CONTROL */
#define BOARD_USART_RTS_PIN 5 /* For USE_USART_HW_FLOW_CONTROL */

// LED Definitions
//  Active low polarity for LEDs
#define BOARD_LED_ACTIVE_LOW true

// List of GPIO pins for the LEDs on the board.  This is based on the amount of LEDs on the board
// MBT02 only has 1 LED
#define BOARD_LED_PIN_LIST \
    {                      \
        31, 30, 27}

// BUZZER
#define BUZZER_PIN 19

// I2C settings
#define USE_I2C1
#define BOARD_I2C_SDA_PIN 24
#define BOARD_I2C_SCL_PIN NRF_GPIO_PIN_MAP(1, 4)
#define BOARD_I2C_PULLUP 1

// LIS3DH
#define LIS3DH_INTERRUPT_1_PIN 7

// SPI
#define USE_SPI0
#define BOARD_SPI_SCK_PIN 3
#define BOARD_SPI_MOSI_PIN 8
#define BOARD_SPI_MISO_PIN 29

// DW1000
#define DW_CS_PIN NRF_GPIO_PIN_MAP(1, 6)
#define DW_IRQ_PIN NRF_GPIO_PIN_MAP(1, 2)
// #define DW1000_WAKEUP_PIN 20 // Active high
#define DW_RST_PIN 25 // Active Low

// List of GPIO pins
#define BOARD_GPIO_PIN_LIST     \
    {                           \
        NRF_GPIO_PIN_MAP(1, 2), \
        16}

// User friendly name for GPIOs (IDs mapped to the BOARD_GPIO_PIN_LIST table)

#define BOARD_GPIO_ID_DW_IRQ 0 // mapped to
#define BOARD_GPIO_ID_LIS3DH 1 // mapped to

// The board supports DCDC (#define BOARD_SUPPORT_DCDC)
// Since SDK v1.2 (bootloader > v7) this option has been move to
// board/<board_name>/config.mk. Set board_hw_dcdc to yes to enable DCDC.
#ifdef BOARD_SUPPORT_DCDC
#error This option has been moved to board/<board_name>/config.mk
#endif

#endif /* BOARD_C3W_BOARD_H_ */
