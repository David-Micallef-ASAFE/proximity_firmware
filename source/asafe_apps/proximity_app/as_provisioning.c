/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_provisioning.c
 * \brief   This file handles all the provisioning logic
 */

#include <stdlib.h>

#include "api.h"
#include "node_configuration.h"
#include "shared_data.h"
#include "app_scheduler.h"

#include "provisioning.h"
#include "local_provisioning.h"
#include "stack_state.h"

#include "as_provisioning.h"
#include "as_proximity.h"
#include "as_protocol.h"
#include "as_storage.h"

#define DEBUG_LOG_MODULE_NAME "AS PROV"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

#define PROVISIONING_ATTEMPT_PERIOD_S (3600 * 24)

#define PROVISIONING_WATCHDOG_PERIOD_S (60)

#define PROVISIONING_PID (0x35B3F081)

extern asProximitySettings_t asProximitySettings;

/*Structure to store the provisioning settings stored in secure storage*/
static provisioning_settings_t m_prov_settings;

// Pre-shared keys.  This is specifically set to 0 here and it is initialized from secure storage when provisioning is required
static local_provisioning_psk_t m_psk = {
    // 4 bytes id
    .id = 0x00000000,
    // 32 bytes psk
    .psk = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

// static uint32_t restartProvisioning()
// {
//     // There is no way to restart the provisioning attempt and a restart of the node is required
//     lib_state->stopStack();
//     return APP_SCHEDULER_STOP_TASK;
// }

/**
 * \brief   Callback called at the end of the provisioning attempt
 */
static bool on_joining_end_cb(bool success)
{

    if (success)
    {
        LOG(LVL_INFO, "Provisioning Success - Restarting");
        // At this point we will have a restart so no need to add any actions here.
    }
    else
    {
        LOG(LVL_INFO, "Provisioning Failure");
    }
    LOG_FLUSH(LVL_INFO);

    /*
    By returning false we indicate to the provisioning library that we
    do not want to restart the stack if the provisioning fails
     */
    return true;
}

// This task runs separate from the provisioning library and will restart the device (to initiate a new provisioning)
// static uint32_t provisioningWatchdogTask()
// {
//     if (!Local_provisioning_is_provisioned())
//     {
//         LOG(LVL_INFO, "Checking Provisioning %d", lib_time->getTimestampS())
//         if (lib_time->getTimestampS() >= PROVISIONING_ATTEMPT_PERIOD_S)
//         {
//             restartProvisioning();
//         }
//         return (PROVISIONING_WATCHDOG_PERIOD_S * 1000);
//     }
//     else
//     {
//         return APP_SCHEDULER_STOP_TASK;
//     }
// }

void asProvisioningInit()
{
    /*Read the keys from storage*/
    m_psk.id = asProximitySettings.pskId; // In the original firmware it was fixed.

    LOG(LVL_INFO, "Provisioning using ASAFE Storage Keys");
    // We have a key coming from ASAFE Storage
    memcpy(m_psk.psk, asProximitySettings.pskKey, 32);
    local_provisioning_config_t conf = {
        .timeout_s = 120};
    Local_provisioning_init(&m_psk, asProvisioningOnProvisionProxyEnabledCB, &conf);

    /*First create a task that will act as our watchdog*/
    // App_Scheduler_addTask_execTime(provisioningWatchdogTask, APP_SCHEDULER_SCHEDULE_ASAP, 500);
}

/**
 * \brief   Provisioning Init: handles starting provisioning attempts
 */
void asProvisioningStart()
{
    // Here we will setup how provisioning is started.
    if (!Local_provisioning_is_provisioned())
    {
        LOG(LVL_INFO, "Starting Provisioning");
        // Local_provisioning_init(&m_psk, asProvisioningOnProvisionProxyEnabledCB, &conf);
        Local_provisioning_start_joining(NULL, on_joining_end_cb);
    }
}

/**
 * \brief   Provisioning Proxy Enabled Callback:
 *          This function is called by the provisioning library to indicate that provisioning can start or stop.
 */
void asProvisioningOnProvisionProxyEnabledCB(bool enabled)
{
    LOG(LVL_INFO, "PPCB Callback");
}

void asProvioningUpdateKeys(const asProtoValue_t *value)
{
    if (value->numBytes == 32)
    {
        LOG(LVL_INFO, "Updating P Keys");
        memcpy(&asProximitySettings.pskKey, value->bytes, value->numBytes);
        asStorageWriteProximitySettings();
    }
    else
    {
        LOG(LVL_INFO, "Error updating PKey.  Not Valid");
    }
}

void asProvisioningLoadKeys(asProximitySettings_t *settings)
{
    // First we try to read it from the secure area provided in firmware
    provisioning_ret_e ret;
    ret = Provisioning_settings_read(&m_prov_settings);
    if (ret != PROV_RET_OK)
    {
        /*Its normal to get here as the settings are wiped once read*/
        LOG(LVL_ERROR, "Error reading provisioning settings %d", ret);
        return;
    }
    /*If we are here it means we have settings and we need to store in our secure storage*/
    // Store the key in secure storage as this will be wiped.
    // Note we copy 32 bytes because enc_key comes after auth_key so we can use it to overflow copy from both auth_key and enc_key into the pskKey.
    memcpy(settings->pskKey, m_prov_settings.prov_keys.auth_key, 32);

    /* -pid is stored as raw bytes in hex-string order (big-endian), e.g. "2A89C376" -> 2A 89 C3 76 */
    if (m_prov_settings.prov_uid_len == sizeof(uint32_t))
    {
        const uint8_t *uid = m_prov_settings.prov_uid.uid;
        settings->pskId = ((uint32_t)uid[0] << 24) |
                          ((uint32_t)uid[1] << 16) |
                          ((uint32_t)uid[2] << 8) |
                          ((uint32_t)uid[3]);
        LOG(LVL_INFO, "PSK Id from prov settings: 0x%08x", (unsigned int)settings->pskId);
    }
    else
    {
        LOG(LVL_WARNING, "Unexpected prov UID length %d", m_prov_settings.prov_uid_len);
    }
}
