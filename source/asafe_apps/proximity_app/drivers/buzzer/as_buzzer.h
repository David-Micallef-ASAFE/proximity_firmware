/* Copyright 2019 Redlore. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file led.h
 *
 */

#ifndef __AS_BUZZER_H_
#define __AS_BUZZER_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

typedef enum
{
    BUZZER_ACTION_ON,
    BUZZER_ACTION_OFF,
    BUZZER_ACTION_DELAY,       // 500mS
    BUZZER_ACTION_DELAY_SHORT, // 200mS
    BUZZER_ACTION_DELAY_LONG   // 1000mS
} buzzerAction_e;

void asBuzzerInit(void);
void asBuzzerBeep(void);
void asBuzzerBeepLong();
void asBuzzerBeepShort();
void asBuzzerBeeps(int8_t beeps);
bool asBuzzerActionOngoing();

#endif /* __AS_BUZZER_H_ */
