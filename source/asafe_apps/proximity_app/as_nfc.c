/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file ASAFE NFC.c
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

#include "as_manufacturing.h"
#include "as_proximity.h"
#include "as_led.h"
#include "as_buzzer.h"

#define DEBUG_LOG_MODULE_NAME "NFC"
/** To activate logs, configure the following line with "LVL_INFO". */
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

/** Time the field must be held to trigger the short hold action */
#define NFC_SHORT_HOLD_MS 1000
/** Time the field must be held to trigger the long hold action */
#define NFC_LONG_HOLD_MS 3000
/** How often the field is sampled while a hold is in progress */
#define NFC_POLL_PERIOD_MS 100
/** Field drop-outs shorter than this are ignored (readers may pulse the field) */
#define NFC_FIELD_LOST_TOLERANCE_MS 300

volatile bool nfc_field_detected = false;

static volatile bool holdActive = false;
static volatile bool fieldPresent = false;
static bool fieldDetectedReported = false;
static bool shortHoldReached = false;
static bool longHoldReached = false;
static app_lib_time_timestamp_hp_t fieldStart;
static app_lib_time_timestamp_hp_t fieldLastSeen;

extern asProximitySettings_t asProximitySettings;

/**
 * \brief   Called once per hold as soon as the field is detected.
 *          Runs from the hold task (not the IRQ) so feedback functions are safe to call.
 */
static void fieldDetected(void)
{
    LOG(LVL_INFO, "Field detected");
// TODO: User feedback
#ifdef APP_BUZZER
    asBuzzerBeepShort(true);
#endif
}

/**
 * \brief   Called once per hold, while the field is still present, as soon as the
 *          short hold time is reached. Use it to give the user feedback.
 */
static void shortAchieved(void)
{
    LOG(LVL_INFO, "Short hold reached");
    if (asProximitySettings.deviceManagementEnabled)
    {
        LED_FLASH_SHORT(0, 1);
#ifdef APP_BUZZER
        asBuzzerBeepShort(false);
#endif
    }

    // TODO: User feedback
}

/**
 * \brief   Called once per hold, while the field is still present, as soon as the
 *          long hold time is reached. Use it to give the user feedback.
 */
static void longAchieved(void)
{
    LOG(LVL_INFO, "Long hold reached");
    if (asProximitySettings.deviceManagementEnabled)
    {
        LED_FLASH_SHORT(0, 2);
#ifdef APP_BUZZER
        asBuzzerBeepDoubleShort();
#endif
    }
    // TODO: User feedback
}

static void nfcShortHoldAction(void)
{
    LOG(LVL_INFO, "Short hold");
    if (asProximitySettings.deviceManagementEnabled)
    {
        asManufacturingEnable();
    }
}

static void nfcLongHoldAction(void)
{
    LOG(LVL_INFO, "Long hold");
    // TODO: Long hold action placeholder
    if (asProximitySettings.deviceManagementEnabled)
    {
        asProximityToggleOperatingMode();
    }
}

/**
 * \brief   Runs while the field is present and fires the hold actions.
 *          shortAchieved/longAchieved give feedback as each time is reached.
 *          On release, the long action runs if held 3s or more, otherwise the
 *          short action runs if held 1s or more, so only one action ever runs per hold.
 */
static uint32_t nfcHoldTask(void)
{
    app_lib_time_timestamp_hp_t now = lib_time->getTimestampHp();

    if (!fieldDetectedReported)
    {
        fieldDetectedReported = true;
        fieldDetected();
    }

    // FIELDPRESENT is only reliable once NFCT is activated (done in the IRQ on FIELDDETECTED).
    // Checking it as well as the event flag covers errata 79 (false FIELDDETECTED with no FIELDLOST)
    if (fieldPresent &&
        (NRF_NFCT->FIELDPRESENT & (NFCT_FIELDPRESENT_FIELDPRESENT_Msk | NFCT_FIELDPRESENT_LOCKDETECT_Msk)))
    {
        fieldLastSeen = now;
    }

    uint32_t heldMs = lib_time->getTimeDiffUs(fieldLastSeen, fieldStart) / 1000;

    if (!shortHoldReached && heldMs >= NFC_SHORT_HOLD_MS)
    {
        shortHoldReached = true;
        shortAchieved();
    }

    if (!longHoldReached && heldMs >= NFC_LONG_HOLD_MS)
    {
        longHoldReached = true;
        longAchieved();
    }

    if ((lib_time->getTimeDiffUs(now, fieldLastSeen) / 1000) < NFC_FIELD_LOST_TOLERANCE_MS)
    {
        return NFC_POLL_PERIOD_MS;
    }

    // Field released
    LOG(LVL_INFO, "Field held for %u ms", heldMs);
    if (heldMs >= NFC_LONG_HOLD_MS)
    {
        nfcLongHoldAction();
    }
    else if (heldMs >= NFC_SHORT_HOLD_MS)
    {
        nfcShortHoldAction();
    }

    // Make sure we are back sensing in case FIELDLOST never fired
    fieldPresent = false;
    NRF_NFCT->TASKS_SENSE = 1;

    holdActive = false;
    return APP_SCHEDULER_STOP_TASK;
}

#pragma GCC push_options
#pragma GCC target("general-regs-only")
/**
 * \brief   Function to handle the NFC Interrupt.
 */
void __attribute__((__interrupt__)) IRQ_handler(void)
{
    // LOG(LVL_INFO, "INT-D");
    if (NRF_NFCT->EVENTS_FIELDDETECTED)
    {
        NRF_NFCT->EVENTS_FIELDDETECTED = 0;
        (void)NRF_NFCT->EVENTS_FIELDDETECTED; // flush write

        // FIELDPRESENT is not valid in SENSE state, so activate NFCT to track the field.
        // FIELDLOST then fires on removal and SHORTS returns NFCT to SENSE.
        NRF_NFCT->TASKS_ACTIVATE = 1;
        fieldPresent = true;
        nfc_field_detected = true; // <-- someone scanned

        // Start timing a new hold (a re-detect within the tolerance continues the current one)
        if (!holdActive)
        {
            LOG(LVL_INFO, "Enable Hold");
            fieldStart = lib_time->getTimestampHp();
            fieldLastSeen = fieldStart;
            fieldDetectedReported = false;
            shortHoldReached = false;
            longHoldReached = false;
            holdActive = true;
            // Run ASAP so fieldDetected() fires straight away, then the task polls every NFC_POLL_PERIOD_MS
            if (App_Scheduler_addTask_execTime(nfcHoldTask, APP_SCHEDULER_SCHEDULE_ASAP, 500) != APP_SCHEDULER_RES_OK)
            {
                LOG(LVL_ERROR, "No scheduler task available");
                holdActive = false;
            }
        }
    }

    if (NRF_NFCT->EVENTS_FIELDLOST)
    {
        NRF_NFCT->EVENTS_FIELDLOST = 0;
        (void)NRF_NFCT->EVENTS_FIELDLOST;
        fieldPresent = false;
        // SHORTS below re-arms SENSE automatically
    }
}

void asNFCInit(void)
{
    NRF_NFCT->SHORTS = NFCT_SHORTS_FIELDLOST_SENSE_Msk; // re-arm after each scan
    NRF_NFCT->INTENSET = NFCT_INTENSET_FIELDDETECTED_Msk |
                         NFCT_INTENSET_FIELDLOST_Msk;

    NRF_NFCT->ERRORSTATUS = NFCT_ERRORSTATUS_FRAMEDELAYTIMEOUT_Msk;
    NRF_NFCT->TASKS_SENSE = 1;

    lib_system->clearPendingFastAppIrq(NFCT_IRQn);
    lib_system->enableAppIrq(true, NFCT_IRQn, APP_LIB_SYSTEM_IRQ_PRIO_LO, IRQ_handler);

    LOG(LVL_INFO, "NFC Enabled");
}
