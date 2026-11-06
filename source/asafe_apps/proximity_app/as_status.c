/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_status.c
 * \brief   This file handles all the klogic related to status updates
 */

#include <stdlib.h>

#include "api.h"
#include "node_configuration.h"
#include "shared_data.h"
#include "app_scheduler.h"

#include "hal_api.h"

#include "as_types.h"
#include "as_wirepas.h"
#include "as_proximity.h"
#include "as_battery.h"

#define DEBUG_LOG_MODULE_NAME "AS STATUS"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

#define STATUS_TASK_PERIOD_SECONDS (10)
// #define STATUS_UPDATE_PERIOD_SECONDS (3600 * 24)
#define STATUS_UPDATE_PERIOD_SECONDS (10) // For Testing only

statusMsg_t statusMessage;

extern app_global_functions_t *global_functions;

extern asProximitySettings_t asProximitySettings;

static uint32_t asGetStackVersion()
{
    app_firmware_version_t firmwareVersion = global_functions->getStackFirmwareVersion();
    return firmwareVersion.version;
}

static uint32_t asGetSoftwareVersion()
{
    uint32_t version = VER_MAJOR;
    version <<= 8;
    version += VER_MINOR;
    version <<= 8;
    version += VER_MAINT;
    version <<= 8;
    version += VER_DEV;
    return version;
}

static uint32_t asStatusTask()
{
    static uint16_t sequence = 0;
    static uint32_t timestampLastStatusTransmit = 0;
    uint8_t srcEndpoint; // This is based on the device type

    uint8_t wirepasRole;

    // if (timestampLastStatusTransmit == 0 ||
    //     (lib_time->getTimestampS() - timestampLastStatusTransmit) > asProximitySettings.heartbeatPeriodSeconds)
    if (timestampLastStatusTransmit == 0 ||
        (lib_time->getTimestampS() - timestampLastStatusTransmit) > 10)
    {
        LOG(LVL_INFO, "Sending status update");
        /*Update the timestamp*/
        timestampLastStatusTransmit = lib_time->getTimestampS();

        /*Get the current Wirepas Role*/
        lib_settings->getNodeRole(&wirepasRole);

        statusMessage.deviceType = ASAFE_DEVICE_TYPE;
        switch (ASAFE_DEVICE_TYPE)
        {
        case DEVICE_TYPE_MINEW_MBT02:
        case DEVICE_TYPE_MINEW_C10:
        case DEVICE_TYPE_MINEW_E9:
        case DEVICE_TYPE_KKM_K4W:
        case DEVICE_TYPE_KKM_C3W:
            statusMessage.deviceRole = DEVICE_ROLE_TAG;
            break;
        case DEVICE_TYPE_ASAFE_JIG:
        default:
            statusMessage.deviceRole = DEVICE_ROLE_ANCHOR;
            break;
        }
        statusMessage.sequence = sequence++;
        statusMessage.softwareVersion = asGetSoftwareVersion();
        statusMessage.wirepasStackVersion = asGetStackVersion();
        statusMessage.wirepasRole = wirepasRole;
        // Up until 1.0.1.31 we were still using App_area_id
        statusMessage.appAreaId = APP_AREA_ID;

        /*Source Endpoint*/
        switch (ASAFE_DEVICE_TYPE)
        {
        case DEVICE_TYPE_MINEW_MBT02:
        case DEVICE_TYPE_MINEW_C10:
        case DEVICE_TYPE_MINEW_E9:
            srcEndpoint = ASAFE_ENDPOINT_SRC_SR;
            break;
        case DEVICE_TYPE_KKM_K4W:
        case DEVICE_TYPE_KKM_C3W:
            srcEndpoint = ASAFE_ENDPOINT_SRC_HR_TAG;
            break;
        default:
            srcEndpoint = ASAFE_ENDPOINT_SRC_SR;
            break;
        }

        statusMessage.vbat = asBatteryGetMv();

        asWirepasSend((uint8_t *)&statusMessage, sizeof(statusMessage), srcEndpoint, ASAFE_ENDPOINT_STATUS_DST, APP_LIB_DATA_QOS_NORMAL);
    }
    LOG(LVL_INFO, "Status update");
    // commSend((uint8_t *)&statusMessage, sizeof(statusMsg_t), ENDPOINT_UPLINK_STATUS_SOURCE, ENDPOINT_UPLINK_STATUS_DESTINATION, false, 0);
    return (STATUS_TASK_PERIOD_SECONDS * 1000);
}

/**
 * \brief   This function initializes the status system.  This is typically called after the node has been provisioned
 */
void asStatusInit()
{
    // Add a task to send a heartbeat every X Seconds
    statusMessage.sequence = 0;

    App_Scheduler_addTask_execTime(asStatusTask, STATUS_TASK_PERIOD_SECONDS * 1000, 500);
}
