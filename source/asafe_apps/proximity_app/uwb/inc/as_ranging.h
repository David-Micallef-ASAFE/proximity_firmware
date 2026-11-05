/* Copyright 2022 Redlore. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

#ifndef __AS_RANGING_H_
#define __AS_RANGING_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"

void rangingInit();
uint8_t rangingSlotSequence();
uint32_t rangingSlotReady();
void rangingAddAnchor(uint32_t address, uint32_t rst, uint8_t rssi, uint8_t losQuality);
void rangingSlotRSS(uint32_t address, uint32_t rss, uint8_t sequence, uint8_t rssi, uint8_t losQuality);
void rangingSlotRSE(uint32_t rse);
void rangingStart(bool motionCondition);
bool rangingCommsReceived();

#endif //__RANGING_H_