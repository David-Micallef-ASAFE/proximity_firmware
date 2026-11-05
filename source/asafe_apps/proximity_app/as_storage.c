/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_storage.c
 * \brief   This file handles all the secure storage
 */

#include <stdlib.h>

#include "api.h"
#include "node_configuration.h"
#include "shared_data.h"
#include "app_scheduler.h"

#include "app_secure_storage.h"

#include "as_proximity.h"
#include "as_provisioning.h"
#include "as_types.h"

#define DEBUG_LOG_MODULE_NAME "AS STORAGE"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

#define APP_PERSISTENT_MEMORY_AREA_ID 0x8AE573BA // Default app_persistent memory area id
#define RECORD_SIZE 16

#define AS_RTLS_DEFAULT_RST_SLOT_OFFSET 0

#define DEFAULT_PSK_ID (0x35B3F081)

#define DEFAULT_HEARTBEAT_PERIOD_SECONDS 3600

#define DEFAULT_MODE AS_MODE_INVENTORY;
// #define DEFAULT_MODE AS_MODE_RUN;

/*
Not yet an ASAFE Standard however suggestion is:
TAG will have 0xXXXXYYYY where XXXX refers to the application (e.g. Proximity) and YYYY refers to the record type of that application
*/

#define AS_PROXIMITY_RECORD_TAG 0x00010000

extern asProximitySettings_t asProximitySettings;

/** Test record structure */
typedef struct asProximityRecord
{
    uint32_t areaId;
    uint32_t tag;
    uint8_t data[sizeof(asProximitySettings_t)];
    size_t len;
    uint32_t flags;
} asProximityRecord_t;

asProximityRecord_t asProximityRecord;

void asStorageInit()
{
    asProximitySettings_t *settings = (asProximitySettings_t *)&asProximityRecord.data[0];
    /** Create the secure storage area */
    if (App_SecureStorage_init(APP_PERSISTENT_MEMORY_AREA_ID) != APP_SECURE_STORAGE_RES_OK)
    {
        LOG(LVL_ERROR, "SecureStorage init failed\n");
        return;
    }

    /** Read the proximity data back from secures storage*/
    asProximityRecord.areaId = APP_PERSISTENT_MEMORY_AREA_ID;
    asProximityRecord.tag = AS_PROXIMITY_RECORD_TAG;
    asProximityRecord.len = sizeof(asProximitySettings_t);
    if ((App_SecureStorage_read(asProximityRecord.areaId,
                                asProximityRecord.tag,
                                &asProximityRecord.data[0],
                                &asProximityRecord.len) != APP_SECURE_STORAGE_RES_OK) ||
        (asProximityRecord.len != sizeof(asProximitySettings_t)))
    {
        LOG(LVL_INFO, "Loading Default Settings");
        // settings->pskId = DEFAULT_PSK_ID;
        memset(settings->pskKey, 0, 32); // Initialize as all 0
        // Get the keys if they are in the app_setup secure storage otherwise do nothing
        asProvisioningLoadKeys(settings);
        settings->heartbeatPeriodSeconds = DEFAULT_HEARTBEAT_PERIOD_SECONDS;
        // settings->mode = DEFAULT_MODE;
        switch (ASAFE_DEVICE_TYPE)
        {
        case DEVICE_TYPE_ASAFE_ANCHOR_HR:
        case DEVICE_TYPE_ASAFE_ASSET_TAG_HR_WITH_RETRIEVAL:
        case DEVICE_TYPE_ASAFE_ASSET_TAG_SR_LARGE:
            settings->mode = AS_MODE_INVENTORY;
            break;
        default:
            settings->mode = AS_MODE_RUN;
            break;
        }
        // Due to the OTAP update we will make this dynamic based on device type.
        settings->deviceManagementEnabled = true;
        settings->startInManufacturing = false;
#ifdef APP_UWB
        settings->uwbOperationEnabled = false;
#endif
        /*If we are here it means the record is not found or has incorrect length - Load default */
#ifdef AS_RTLS_ROLE_TAG
        settings->updateRateSeconds = AS_RTLS_DEFAULT_UPDATE_RATE_SECONDS;
        settings->updateRateMotionSeconds = AS_RTLS_DEFAULT_UPDATE_RATE_MOTION_SECONDS;
#endif
#ifdef AS_RTLS_ROLE_ANCHOR
        settings->rstSlotOffset = AS_RTLS_DEFAULT_RST_SLOT_OFFSET;
#endif
        asProximityRecord.len = sizeof(asProximitySettings_t);
        if (App_SecureStorage_write(APP_PERSISTENT_MEMORY_AREA_ID,
                                    asProximityRecord.tag,
                                    &asProximityRecord.data[0],
                                    asProximityRecord.len,
                                    asProximityRecord.flags) != APP_SECURE_STORAGE_RES_OK)
        {
            LOG(LVL_ERROR, "SecureStorage write failed\n");
        }
        else
        {
            LOG(LVL_INFO, "SecureStorage write succeeded\n");
        }
        // Read the default settings into the asProximitySettings structure so that the rest of the application can use it
        memcpy(&asProximitySettings, settings, sizeof(asProximitySettings_t));
    }
    else
    {
        /*We have successfully read the record - Update the settings*/
        memcpy(&asProximitySettings, settings, sizeof(asProximitySettings_t));
        LOG(LVL_INFO, "Proximity Settings read succeeded\n");
    }
    /*At this point all the settings have been read from storage and the modules in the system can start using them*/
    LOG(LVL_INFO, "Node Address: %u", getUniqueAddress());
    LOG(LVL_INFO, "Proximity Settings:");
    LOG(LVL_INFO, "  PSK Id: 0x%08x", (unsigned int)asProximitySettings.pskId);
    LOG(LVL_INFO, "  PSK Key:");
    LOG_BUFFER(LVL_INFO, asProximitySettings.pskKey, sizeof(asProximitySettings.pskKey));
    LOG(LVL_INFO, "  Heartbeat Period: %u s", (unsigned int)asProximitySettings.heartbeatPeriodSeconds);
    LOG(LVL_INFO, "  Mode: %d (%s)", asProximitySettings.mode, asProximitySettings.mode == AS_MODE_RUN ? "Run" : "Inventory");
#ifdef AS_RTLS_ROLE_ANCHOR
    LOG(LVL_INFO, "  RST Slot Offset: %u", (unsigned int)asProximitySettings.rstSlotOffset);
#endif
#ifdef AS_RTLS_ROLE_TAG
    LOG(LVL_INFO, "  Update Rate: %u s", (unsigned int)asProximitySettings.updateRateSeconds);
    LOG(LVL_INFO, "  Update Rate Motion: %u s", (unsigned int)asProximitySettings.updateRateMotionSeconds);
#endif
    LOG_FLUSH(LVL_INFO);
    /*Print the settings*/
}

/*Use this function to write the proximity settings to secure storage*/
void asStorageWriteProximitySettings()
{
    asProximitySettings_t *settings = (asProximitySettings_t *)&asProximityRecord.data[0];
    memcpy(settings, &asProximitySettings, sizeof(asProximitySettings_t));
    asProximityRecord.len = sizeof(asProximitySettings_t);
    if (App_SecureStorage_write(APP_PERSISTENT_MEMORY_AREA_ID,
                                asProximityRecord.tag,
                                &asProximityRecord.data[0],
                                asProximityRecord.len,
                                asProximityRecord.flags) != APP_SECURE_STORAGE_RES_OK)
    {
        LOG(LVL_ERROR, "SecureStorage write failed\n");
    }
    else
    {
        LOG(LVL_INFO, "SecureStorage write succeeded\n");
    }
}
