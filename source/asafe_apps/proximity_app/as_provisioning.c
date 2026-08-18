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

#define DEBUG_LOG_MODULE_NAME "AS PROV"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

#define PROVISIONING_ATTEMPT_PERIOD_S 300

// Pre-shared keys, shared with the local_provisioning_tool.py script
static local_provisioning_psk_t m_psk = {
    // EXAMPLES KEY ID AND PRESHARED KEY TO BE REPLACED
    // In production applications, store these in secure storage instead of
    // hardcoding here.
    // See wms_secure_storage.h. Hardcoding results in the data being stored as
    // plaintext on the flash.
    // 4 bytes id
    .id = 0x35B3F081,
    // 32 bytes psk
    .psk = {
        0xe8, 0x57, 0x08, 0x00, 0x39, 0x3a, 0x58, 0x71,
        0x18, 0x86, 0x0f, 0x15, 0x5f, 0x1c, 0x8e, 0x22,
        0x25, 0xd2, 0xbd, 0xed, 0xb2, 0xa5, 0x56, 0x82,
        0xe0, 0xda, 0xf2, 0xb9, 0xdb, 0x3b, 0xf0, 0xcf}};

static local_provisioning_config_t conf = {.timeout_s = 20};

static uint32_t restartProvisioning()
{
    // There is no way to restart the provisioning attempt and a restart of the node is required
    Local_provisioning_reset_node();
    return APP_SCHEDULER_STOP_TASK;
}

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
        LOG(LVL_INFO, "Provisioning Failed, Scheduling Re-Provision in %d Seconds", PROVISIONING_ATTEMPT_PERIOD_S);
        App_Scheduler_addTask_execTime(restartProvisioning, PROVISIONING_ATTEMPT_PERIOD_S * 1000, 500);
    }

    /*
    By returning false we indicate to the provisioning library that we
    do not want to restart the stack if the provisioning fails
     */
    return false;
}

void asProvisioningInit()
{
    Local_provisioning_init(&m_psk, asProvisioningOnProvisionProxyEnabledCB, &conf);
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
