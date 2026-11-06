/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_battery.h
 * \brief Battery voltage measurement, independent of how the board wires VBAT.
 *
 * Boards where VDD is connected directly to the battery need nothing in board.h.
 * Boards with an LDO that measure the battery through a resistor divider define:
 *
 *   #define BOARD_VBAT_PIN            5     // GPIO number of the analog input (P0.05 = AIN3)
 *   #define BOARD_VBAT_R_TOP_KOHM     1000  // resistor from VBAT to the ADC pin
 *   #define BOARD_VBAT_R_BOTTOM_KOHM  1000  // resistor from the ADC pin to GND
 */

#ifndef __AS_BATTERY_H_
#define __AS_BATTERY_H_

#include <stdint.h>

/**
 * \brief Prepare the battery measurement (runs SAADC offset calibration on
 *        divider boards). Safe to call more than once.
 */
void asBatteryInit(void);

/**
 * \brief  Measure the battery voltage now.
 * \return Battery voltage in millivolts.
 */
uint16_t asBatteryGetMv(void);

/**
 * \brief  Last value returned by asBatteryGetMv(), without a new measurement.
 * \return Battery voltage in millivolts, 0 if never measured.
 */
uint16_t asBatteryGetLastMv(void);

#endif /* __AS_BATTERY_H_ */
