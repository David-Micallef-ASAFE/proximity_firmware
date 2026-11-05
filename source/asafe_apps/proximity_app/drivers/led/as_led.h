/* Copyright 2019 Redlore. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_led.h
 *
 */

#ifndef __AS_LED_H_
#define __AS_LED_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"
#ifdef APP_LED
#define LED_INIT() asLedInit()
#define LED_RUN_FLASH() asLedRunFlash()
#define LED_FLASH(num) asLedFlash(num)
#define LED_FLASH_SHORT(num, flash_num) asLedFlashShort(num, flash_num)
#define LED_FLASH_SHORT_DELAYED(num) asLedFlashShortDelayed(num)
#define LED_FLASH_LONG(num) asLedFlashLong(num)
#define LED_FLASH_LONG_DELAYED(num) asLedFlashLongDelayed(num)
#define LED_FLASHES(num, flash_num) asLedFlashes(num, flash_num)
#define LED_FLASH_DELAYED(num) asLedFlashDelayed(num)
#else
#define LED_INIT()
#define LED_RUN_FLASH()
#define LED_FLASH(num)
#define LED_FLASH_SHORT(num)
#define LED_FLASH_SHORT_DELAYED(num)
#define LED_FLASH_LONG(num)
#define LED_FLASH_LONG_DELAYED(num)
#define LED_FLASHES(num, flash_num)
#define LED_FLASH_DELAYED(num)
#endif

/*Note that not all devices have 3 leds - The code will automatically not do anything if the led is not available*/
#define LED_INFO 0
#define LED_WARNING 1
#define LED_ERROR 2

/*These are actions that we can queue for the flash task to dequeu and act upon*/
typedef enum
{
    LED_ACTION_ON,
    LED_ACTION_OFF,
    LED_ACTION_DELAY,       // 500mS
    LED_ACTION_DELAY_SHORT, // 200mS
    LED_ACTION_DELAY_LONG,  // 1000mS
    LED_ACTION_DELAY_1S     // 1000mS
} ledAction_e;

typedef struct
{
    uint8_t pin;
    ledAction_e action;
} ledTask_t;

void asLedInit(void);
void asLedRunFlash();
void asLedFlash(uint8_t num);
void asLedFlashLong(uint8_t num);
void asLedFlashShort(uint8_t num, uint8_t flash_num);
void asLedFlashes(uint8_t num, int8_t flash_num);
void asLedFlashShortDelayed(uint8_t num);
void asLedFlashLongDelayed(uint8_t num);
void asLedFlashDelayed(uint8_t num);

#endif /* __RL_LED_H_ */
