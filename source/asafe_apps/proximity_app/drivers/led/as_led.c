/* Copyright 2019 Redlore. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file led.c
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"
#include "as_led.h"

#include "board.h"

/*----------------DEBUG-------------------------------------------*/
#define DEBUG_LOG_MODULE_NAME "AS_LED"
#define DEBUG_LOG_MAX_LEVEL LVL_DEBUG
#include "debug_log.h"
/*----------------DEBUG-------------------------------------------*/

#define LED_SEQUENCE_TIME_MS 100
#define LED_SEQUENCE_TIME_DELAY_MS 500       // used to introduce a delay in the sequence
#define LED_SEQUENCE_TIME_DELAY_SHORT_MS 100 // used to introduce a delay in the sequence
#define LED_SEQUENCE_TIME_DELAY_LONG_MS 800  // used to introduce a delay in the sequence

/** Time needed to execute the periodic work, in us */
#define LED_TASK_EXECUTION_TIME_US 500

/*Queue Size - Ensure that this is never skipped*/
#define LED_QUEUE_SIZE 300

static ledTask_t ledQueue[LED_QUEUE_SIZE];
static int ledQueueIn, ledQueueOut;

/** \brief  Board-dependent LED number to pin mapping */
static const uint8_t pin_map[] = BOARD_LED_PIN_LIST;

/** \brief  Compute number of leds on the board */
#define NUMBER_OF_LEDS (sizeof(pin_map) / sizeof(pin_map[0]))

static void ledQueueInit(void)
{
    ledQueueIn = ledQueueOut = 0;
}

static int ledQueuePut(uint8_t pin, ledAction_e action)
{
    if (ledQueueIn == ((ledQueueOut - 1 + LED_QUEUE_SIZE) % LED_QUEUE_SIZE))
    {
        return -1; /* Queue Full*/
    }

    ledQueue[ledQueueIn].pin = pin;
    ledQueue[ledQueueIn].action = action;

    ledQueueIn = (ledQueueIn + 1) % LED_QUEUE_SIZE;

    return 0; // No errors
}

static bool ledQueueIsEmpty()
{
    return ledQueueIn == ledQueueOut;
}

static int ledQueueGet(ledTask_t *old)
{
    if (ledQueueIn == ledQueueOut)
    {
        return -1; /* Queue Empty - nothing to get*/
    }

    memcpy(old, &ledQueue[ledQueueOut], sizeof(ledTask_t));

    ledQueueOut = (ledQueueOut + 1) % LED_QUEUE_SIZE;

    return 0; // No errors
}

static void led_action(uint8_t pin, ledAction_e action)
{
    if (BOARD_LED_ACTIVE_LOW)
    {
        if (action == LED_ACTION_ON)
        {
            nrf_gpio_pin_clear(pin);
        }
        else
        {
            nrf_gpio_pin_set(pin);
        }
    }
    else
    {
        if (action == LED_ACTION_ON)
        {
            nrf_gpio_pin_set(pin);
        }
        else
        {
            nrf_gpio_pin_clear(pin);
        }
    }
}

/*
 * \brief   This task is actually a periodic task that terminates after the flash queue is emptied
 */
static uint32_t ledTask()
{
    ledTask_t operation;
    // Get the operation from the queue
    if (ledQueueGet(&operation) == -1)
        return APP_SCHEDULER_STOP_TASK; // End of queueu = end of sequence

    // Act
    switch (operation.action)
    {
    case LED_ACTION_ON:
    case LED_ACTION_OFF:
        led_action(operation.pin, operation.action);
        return APP_SCHEDULER_SCHEDULE_ASAP;
    case LED_ACTION_DELAY:
        return LED_SEQUENCE_TIME_DELAY_MS;
    case LED_ACTION_DELAY_SHORT:
        return LED_SEQUENCE_TIME_DELAY_SHORT_MS;
    case LED_ACTION_DELAY_LONG:
        return LED_SEQUENCE_TIME_DELAY_LONG_MS;
    case LED_ACTION_DELAY_1S:
        return 1000;
    }
    return LED_SEQUENCE_TIME_MS;
}

void asLedInit(void)
{
    uint8_t i;
    // Init the flash queue
    ledQueueInit();
    // Initialize all led pins as outputs and turn them off
    for (i = 0; i < NUMBER_OF_LEDS; i++)
    {
        nrf_gpio_cfg_output(pin_map[i]);
        led_action(pin_map[i], LED_ACTION_OFF);
    }
}

/*This function will flash all the leds on the board*/
void asLedRunFlash()
{
    uint8_t i;
    bool ongoingOperation = !ledQueueIsEmpty();

    LOG(LVL_INFO, "Led run flash\n");
    // Initialize all led pins as outputs and turn them off
    for (i = 0; i < NUMBER_OF_LEDS; i++)
    {
        ledQueuePut(pin_map[i], LED_ACTION_ON);
        ledQueuePut(pin_map[i], LED_ACTION_DELAY);
        ledQueuePut(pin_map[i], LED_ACTION_OFF);
        ledQueuePut(pin_map[i], LED_ACTION_DELAY);
    }
    if (!ongoingOperation)
    {
        App_Scheduler_addTask_execTime(ledTask, APP_SCHEDULER_SCHEDULE_ASAP, LED_TASK_EXECUTION_TIME_US);
    }
}

/*This function will flash the led with the num.   Note num is not a pin number*/
void asLedFlash(uint8_t num)
{
    bool ongoingOperation = !ledQueueIsEmpty();
    if (num < NUMBER_OF_LEDS)
    {
        ledQueuePut(pin_map[num], LED_ACTION_ON);
        ledQueuePut(pin_map[num], LED_ACTION_DELAY);
        ledQueuePut(pin_map[num], LED_ACTION_OFF);
        if (!ongoingOperation)
        {
            App_Scheduler_addTask_execTime(ledTask, APP_SCHEDULER_SCHEDULE_ASAP, LED_TASK_EXECUTION_TIME_US);
        }
    }
}

/*This function will flash the led with the num.   Note num is not a pin number*/
void asLedFlashes(uint8_t num, int8_t flash_num)
{
    bool ongoingOperation = !ledQueueIsEmpty();
    if (num < NUMBER_OF_LEDS && ((flash_num * 4) < LED_QUEUE_SIZE))
    {
        while (flash_num-- > 0)
        {
            ledQueuePut(pin_map[num], LED_ACTION_ON);
            ledQueuePut(pin_map[num], LED_ACTION_DELAY_SHORT);
            ledQueuePut(pin_map[num], LED_ACTION_OFF);
            ledQueuePut(pin_map[num], LED_ACTION_DELAY_LONG);
        }
        if (!ongoingOperation)
        {
            App_Scheduler_addTask_execTime(ledTask, APP_SCHEDULER_SCHEDULE_ASAP, LED_TASK_EXECUTION_TIME_US);
        }
    }
    else
    {

        LOG(LVL_ERROR, "LED Queue size low for operation\n");
        return;
    }
}

void asLedFlashShort(uint8_t num, uint8_t flash_num)
{
    bool ongoingOperation = !ledQueueIsEmpty();
    if (num < NUMBER_OF_LEDS)
    {
        while (flash_num-- > 0)
        {
            ledQueuePut(pin_map[num], LED_ACTION_ON);
            ledQueuePut(pin_map[num], LED_ACTION_DELAY_SHORT);
            ledQueuePut(pin_map[num], LED_ACTION_OFF);
            ledQueuePut(pin_map[num], LED_ACTION_DELAY_SHORT);
        }
        if (!ongoingOperation)
        {
            App_Scheduler_addTask_execTime(ledTask, APP_SCHEDULER_SCHEDULE_ASAP, LED_TASK_EXECUTION_TIME_US);
        }
    }
}

void asLedFlashShortDelayed(uint8_t num)
{
    bool ongoingOperation = !ledQueueIsEmpty();
    if (num < NUMBER_OF_LEDS)
    {
        ledQueuePut(pin_map[num], LED_ACTION_DELAY_1S);
        ledQueuePut(pin_map[num], LED_ACTION_ON);
        ledQueuePut(pin_map[num], LED_ACTION_DELAY_SHORT);
        ledQueuePut(pin_map[num], LED_ACTION_OFF);
        if (!ongoingOperation)
        {
            App_Scheduler_addTask_execTime(ledTask, APP_SCHEDULER_SCHEDULE_ASAP, LED_TASK_EXECUTION_TIME_US);
        }
    }
}

void asLedFlashLong(uint8_t num)
{
    bool ongoingOperation = !ledQueueIsEmpty();
    if (num < NUMBER_OF_LEDS)
    {
        ledQueuePut(pin_map[num], LED_ACTION_ON);
        ledQueuePut(pin_map[num], LED_ACTION_DELAY_LONG);
        ledQueuePut(pin_map[num], LED_ACTION_OFF);
        if (!ongoingOperation)
        {
            App_Scheduler_addTask_execTime(ledTask, APP_SCHEDULER_SCHEDULE_ASAP, LED_TASK_EXECUTION_TIME_US);
        }
    }
}

void asLedFlashLongDelayed(uint8_t num)
{
    bool ongoingOperation = !ledQueueIsEmpty();
    if (num < NUMBER_OF_LEDS)
    {
        ledQueuePut(pin_map[num], LED_ACTION_DELAY_1S);
        ledQueuePut(pin_map[num], LED_ACTION_ON);
        ledQueuePut(pin_map[num], LED_ACTION_DELAY_LONG);
        ledQueuePut(pin_map[num], LED_ACTION_OFF);
        if (!ongoingOperation)
        {
            App_Scheduler_addTask_execTime(ledTask, APP_SCHEDULER_SCHEDULE_ASAP, LED_TASK_EXECUTION_TIME_US);
        }
    }
}

void asLedFlashDelayed(uint8_t num)
{
    bool ongoingOperation = !ledQueueIsEmpty();
    if (num < NUMBER_OF_LEDS)
    {
        ledQueuePut(pin_map[num], LED_ACTION_DELAY_SHORT);
        ledQueuePut(pin_map[num], LED_ACTION_ON);
        ledQueuePut(pin_map[num], LED_ACTION_DELAY_SHORT);
        ledQueuePut(pin_map[num], LED_ACTION_OFF);
        if (!ongoingOperation)
        {
            App_Scheduler_addTask_execTime(ledTask, APP_SCHEDULER_SCHEDULE_ASAP, LED_TASK_EXECUTION_TIME_US);
        }
    }
}