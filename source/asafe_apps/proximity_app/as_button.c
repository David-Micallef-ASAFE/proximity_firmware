/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_button.c
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

#include "board.h"

#include "button.h"

#include "as_types.h"
#include "as_wirepas.h"

#include "as_manufacturing.h"

#include "local_provisioning.h"

#include "as_led.h"
#include "as_proximity.h"

#define DEBUG_LOG_MODULE_NAME "BUTTON"
/** To activate logs, configure the following line with "LVL_INFO". */
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

#define BUTTON_MANUFACTURING_IDX 0

/** Time the button must be held to trigger the short hold action */
#define BUTTON_SHORT_HOLD_MS 1000
/** Time the button must be held to trigger the long hold action */
#define BUTTON_LONG_HOLD_MS 3000
/** How often the hold time is checked while the button is pressed */
#define BUTTON_POLL_PERIOD_MS 100

static volatile bool buttonPressed = false;
static bool shortHoldReached = false;
static bool longHoldReached = false;
static app_lib_time_timestamp_hp_t pressStart;
static volatile app_lib_time_timestamp_hp_t pressEnd;

extern asProximitySettings_t asProximitySettings;

/**
 * \brief   Called once per press, while the button is still held, as soon as the
 *          short hold time is reached. Use it to give the user feedback.
 */
static void shortHoldAchieved(void)
{
    LOG(LVL_INFO, "Short hold reached");
    // TODO: User feedback
    if (asProximitySettings.deviceManagementEnabled)
    {
        LED_FLASH_SHORT(0, 1);
    }
}

/**
 * \brief   Called once per press, while the button is still held, as soon as the
 *          long hold time is reached. Use it to give the user feedback.
 */
static void longHoldAchieved(void)
{
    LOG(LVL_INFO, "Long hold reached");
    // TODO: User feedback
    if (asProximitySettings.deviceManagementEnabled)
    {
        LED_FLASH_SHORT(0, 2);
    }
}

static void shortHold(void)
{
    LOG(LVL_INFO, "Short hold");
    if (asProximitySettings.deviceManagementEnabled)
    {
        asManufacturingEnable();
    }
}

static void longHold(void)
{
    LOG(LVL_INFO, "Long hold");
    // TODO: Long hold action placeholder
    if (asProximitySettings.deviceManagementEnabled)
    {
        asProximityToggleOperatingMode();
    }
}

/**
 * \brief   Runs while the button is pressed and fires the hold actions.
 *          shortHoldAchieved/longHoldAchieved give feedback as each time is reached.
 *          On release, the long action runs if held 3s or more, otherwise the
 *          short action runs if held 1s or more, so only one action ever runs per press.
 */
static uint32_t buttonHoldTask(void)
{
    bool pressed = buttonPressed;
    app_lib_time_timestamp_hp_t end = pressed ? lib_time->getTimestampHp() : pressEnd;
    uint32_t heldMs = lib_time->getTimeDiffUs(end, pressStart) / 1000;

    if (!shortHoldReached && heldMs >= BUTTON_SHORT_HOLD_MS)
    {
        shortHoldReached = true;
        shortHoldAchieved();
    }

    if (!longHoldReached && heldMs >= BUTTON_LONG_HOLD_MS)
    {
        longHoldReached = true;
        longHoldAchieved();
    }

    if (pressed)
    {
        return BUTTON_POLL_PERIOD_MS;
    }

    // Button released
    LOG(LVL_INFO, "Button held for %u ms", heldMs);
    if (heldMs >= BUTTON_LONG_HOLD_MS)
    {
        longHold();
    }
    else if (heldMs >= BUTTON_SHORT_HOLD_MS)
    {
        shortHold();
    }

    return APP_SCHEDULER_STOP_TASK;
}

/*Note that this will run in the interrupt so it needs to be short*/
static void buttonEvent(uint8_t button_id, button_event_e event)
{
    switch (event)
    {
    case BUTTON_PRESSED:
        LOG(LVL_INFO, "Button pressed");
        pressStart = lib_time->getTimestampHp();
        shortHoldReached = false;
        longHoldReached = false;
        buttonPressed = true;
        if (App_Scheduler_addTask_execTime(buttonHoldTask, BUTTON_POLL_PERIOD_MS, 500) != APP_SCHEDULER_RES_OK)
        {
            LOG(LVL_ERROR, "No scheduler task available");
            buttonPressed = false;
        }
        break;
    case BUTTON_RELEASED:
        LOG(LVL_INFO, "Button released");
        if (buttonPressed)
        {
            pressEnd = lib_time->getTimestampHp();
            buttonPressed = false;
            // Process the release straight away rather than waiting for the next poll
            App_Scheduler_addTask_execTime(buttonHoldTask, APP_SCHEDULER_SCHEDULE_ASAP, 500);
        }
        break;
    }
}

void asButtonInit()
{
    switch (ASAFE_DEVICE_TYPE)
    {
    case DEVICE_TYPE_MINEW_MBT02:
    case DEVICE_TYPE_MINEW_C10:
    case DEVICE_TYPE_MINEW_E9:
    case DEVICE_TYPE_KKM_K4W:
    case DEVICE_TYPE_KKM_C3W:
    case DEVICE_TYPE_ASAFE_JIG:
    case DEVICE_TYPE_ASAFE_ANCHOR_HR:
        Button_register_for_event(BUTTON_MANUFACTURING_IDX, BUTTON_PRESSED, buttonEvent);
        Button_register_for_event(BUTTON_MANUFACTURING_IDX, BUTTON_RELEASED, buttonEvent);
        LOG(LVL_INFO, "Registering Button Event");
        break;
    default:
        LOG(LVL_ERROR, "This device does not have a button");
        break;
    }
}