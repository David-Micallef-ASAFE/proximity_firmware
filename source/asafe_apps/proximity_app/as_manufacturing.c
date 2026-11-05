/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_manufacturing.c
 * \brief   This file handles all the manufacturing logic
 */

#include <stdlib.h>

#include "api.h"
#include "node_configuration.h"
#include "shared_data.h"
#include "app_scheduler.h"

#include "provisioning.h"
#include "local_provisioning.h"
#include "stack_state.h"
#include "as_led.h"
#include "as_ble.h"
#include "as_storage.h"
#include "as_bleBareMetal.h"

#define DEBUG_LOG_MODULE_NAME "MANUFACTURING"
/** To activate logs, configure the following line with "LVL_INFO". */
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

#define MANUFACTURING_TIME_S 5

extern asProximitySettings_t asProximitySettings;

bool manufacturingEnabled;
bool requireRestartPostManufacturing;

static void manufacturingReady()
{
    manufacturingEnabled = false;
    if (requireRestartPostManufacturing)
    {
        lib_state->stopStack();
    }
}

// Typically when a device comes close to the Manufacturing Device, we will enable the manufacturing task
// A device will keep monitoring for the manufacturing device for 1 hour
void asManufacturingEnable()
{
    if (!manufacturingEnabled)
    {
        manufacturingEnabled = true;
        if (lib_state->getStackState() != APP_LIB_STATE_STARTED)
        {
            // We can start manufacturing
            asBLEBareMetalStart(MANUFACTURING_TIME_S, getUniqueAddress(), asProximitySettings.mode, ASAFE_DEVICE_TYPE, manufacturingReady);
            return;
        }
        else
        {
            asProximitySettings.startInManufacturing = true;
            asStorageWriteProximitySettings();
            // Force a restart;
            lib_state->stopStack();
        }
    }
}

void asManufacturingInit()
{
    manufacturingEnabled = false;
    requireRestartPostManufacturing = false;
    if (asProximitySettings.startInManufacturing)
    {
        requireRestartPostManufacturing = true;
        asProximitySettings.startInManufacturing = false;
        asStorageWriteProximitySettings();
        asManufacturingEnable();
    }
}
