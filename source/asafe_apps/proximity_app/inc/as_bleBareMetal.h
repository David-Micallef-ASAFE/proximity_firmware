/* Copyright 2026 Redlore. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_bleBareMetal.h
 *
 * Bare-metal Eddystone-UID advertiser (drives the RADIO directly).
 * Only use while the Wirepas stack is stopped.
 */

#ifndef __AS_BLE_BARE_METAL_H_
#define __AS_BLE_BARE_METAL_H_

#include <stdint.h>
#include <stdbool.h>

#include "api.h"
#include "as_proximity.h"

/* Called once advertising has finished and the radio has been released */
typedef void (*asBLEBareMetalDoneCb_f)(void);

/**
 * Advertise an Eddystone-UID beacon (~3 Hz, channels 37/38/39) for timeSeconds.
 * Blocks (busy-waits on the CPU clock) until done, then calls doneCb (may be NULL).
 *
 * AdvA (random address) as shown by scanners: [serial MSB..LSB]:[mode]:[deviceType]
 *
 * Returns false (without advertising, and without calling doneCb) if the
 * Wirepas stack is running or timeSeconds is 0.
 */
bool asBLEBareMetalStart(uint32_t timeSeconds, uint32_t serialNumber, asMode_e mode, uint8_t deviceType,
                         asBLEBareMetalDoneCb_f doneCb);

#endif /* __AS_BLE_BARE_METAL_H_ */
