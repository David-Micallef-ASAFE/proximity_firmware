/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_positioning.c
 * \brief   This file handles all the positioning logic
 */

#include <stdlib.h>

#include "api.h"
#include "node_configuration.h"
#include "shared_data.h"
#include "app_scheduler.h"

#include "local_provisioning.h"

#include "as_proximity.h"
#include "as_data.h"

#include "stack_state.h"
#include "as_wirepas.h"

#include "as_types.h"
#include "as_proximity.h"

#include "hal_api.h"

#include "as_lis3dh.h"
#include "as_uwbTimer.h"

#include "as_manufacturing.h"
#include "as_storage.h"
#include "as_battery.h"

#include "as_led.h"

#define DEBUG_LOG_MODULE_NAME "AS_POSITIONING"
/** To activate logs, configure the following line with "LVL_INFO". */
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

#define MOTION_TIME_THRESHOLD_SECONDS 10

#define AS_ADDRESS_MANUFACTURING_DEVICE 100 // If a device sees this address within a specific RSSI, it will enable manfuacturing mode
#define AS_MANUFACTURING_MODE_RSSI -50      // If a device sees this address within a specific RSSI, it will enable manfuacturing mode

/*
Note that the ASAFE RTLS supports 2 modes of operation SR and HR.  Most of the implementation here is for the SR mode.
The HR Implementation is separate and injects into this file.

Configuration
Role: AS_RTLS_MODE_ANCHOR or AS_RTLS_MODE_TAG
Mode: MODE_SR or MODE_HR or MODE_HR_SR

ANCHOR Only Settings

TAG Only Settings
Update Period: 10 seconds to 24 Hours
*/

#define TAG_TASK_DUTY_CYCLE_SECONDS 1

asProximitySettings_t asProximitySettings;

#ifdef AS_RTLS_ROLE_TAG

rpeMessage_t rpeMessage;
uint8_t nborIdx;

bool motionDetected;
uint32_t motionDetectedAtTimestamp;

/*Period values*/
uint32_t scanPeriodSeconds;
uint16_t scanPeriodMaxG;
proximityScanReason_e previousScanReason;
proximityScanReason_e currentScanReason;

#ifdef AS_RTLS_MODE_SR

static int compare(const void *s1, const void *s2)
{
    nborInfo_t *nb1 = (nborInfo_t *)s1;
    nborInfo_t *nb2 = (nborInfo_t *)s2;
    if (nb1->normRSSI < nb2->normRSSI)
        return 1;
    if (nb1->normRSSI > nb2->normRSSI)
        return -1;
    return 0;
}

static int8_t isAlreadyInDatabase(const app_lib_state_beacon_rx_t *beacon)
{
    for (uint8_t i = 0; i < nborIdx; i++)
    {
        if (rpeMessage.nbors[i].address == beacon->address)
        {
            return i;
        }
    }
    return -1;
}

static void nborScanReceivedBeaconCB(const app_lib_state_beacon_rx_t *beacon)
{
    int8_t idx;
    Sys_enterCriticalSection();
    LOG(LVL_INFO, "Beacon %d %d %d", beacon->address, beacon->rssi, beacon->txpower);

    /*This section is used specifically for manufacturing*/
    if (beacon->address == AS_ADDRESS_MANUFACTURING_DEVICE && beacon->rssi > AS_MANUFACTURING_MODE_RSSI)
    {
        LOG(LVL_INFO, "Manufacturing Device Detected");
        // Enable manufacturing mode
    }

    idx = isAlreadyInDatabase(beacon);
    if (idx != -1)
    {
        // Check if the RSSI is higher than the previous RSSI.  If so, update the database with the new RSSI and timestamp
        if (beacon->rssi > rpeMessage.nbors[idx].normRSSI)
        {
            rpeMessage.nbors[idx].normRSSI = beacon->rssi;
        }
        rpeMessage.nbors[idx].lastUpdate = lib_time->getTimestampS();
        qsort(rpeMessage.nbors, nborIdx, sizeof(nborInfo_t), compare);
    }
    else
    {
        if (nborIdx < NBOR_SCAN_MAX_NBORS)
        {
            rpeMessage.nbors[nborIdx].address = beacon->address;
            rpeMessage.nbors[nborIdx].normRSSI = beacon->rssi;
            rpeMessage.nbors[nborIdx].txPower = beacon->txpower;
            rpeMessage.nbors[nborIdx].lastUpdate = lib_time->getTimestampS();
            nborIdx++;
            qsort(rpeMessage.nbors, nborIdx, sizeof(nborInfo_t), compare);
        }
        else
        {
            // We will check if the new beacon has a higher RSSI than the lowest RSSI in the database.  If so, we will replace the lowest RSSI with the new beacon
            if (beacon->rssi > rpeMessage.nbors[nborIdx - 1].normRSSI)
            {
                rpeMessage.nbors[nborIdx - 1].address = beacon->address;
                rpeMessage.nbors[nborIdx - 1].normRSSI = beacon->rssi;
                rpeMessage.nbors[nborIdx - 1].txPower = beacon->txpower;
                rpeMessage.nbors[nborIdx - 1].rxPower = 0;
                rpeMessage.nbors[nborIdx - 1].linkReliability = 0;
                rpeMessage.nbors[nborIdx - 1].lastUpdate = lib_time->getTimestampS();
                qsort(rpeMessage.nbors, nborIdx, sizeof(nborInfo_t), compare);
            }
        }
    }
    Sys_exitCriticalSection();
}

static void scanend_cb(app_lib_stack_event_e event, void *param_p)
{
    uint8_t seq = 0;
    if (event == APP_LIB_STATE_STACK_EVENT_SCAN_STOPPED)
    {
        LOG(LVL_INFO, "Scan Ended.  Found %d neighbors", nborIdx);
        // Send the results of the scan

        // Remove the callbacks
        lib_state->setOnBeaconCb(NULL);
        Stack_State_removeEventCb(scanend_cb);

        if (nborIdx > 0)
        {
            // Send the results of the scan to the anchor

            /*Standard RPE V1 - Enable only if you required*/
            // asWirepasSend((uint8_t *)&rpeMessage.nbors, sizeof(nborInfo_t) * nborIdx, ASAFE_ENDPOINT_SRC_SR_RPE, ASAFE_ENDPOINT_PROXIMITY_STATIONARY_DST, APP_LIB_DATA_QOS_NORMAL);

            /*Standard RPE V2*/
            // Create the RPE Message
            rpeMessage.periodHeader.duration = scanPeriodSeconds;
            rpeMessage.periodHeader.maxG = scanPeriodMaxG;
            rpeMessage.periodHeader.sequence = seq++;
            rpeMessage.periodHeader.type = previousScanReason;
            rpeMessage.periodHeader.battery = asBatteryGetMv();

            LOG(LVL_INFO, "Sending RPE Message with %d nbors", nborIdx);
            LOG(LVL_INFO, "Duration %d, MaxG %d, Type %d", scanPeriodSeconds, scanPeriodMaxG, previousScanReason);
            asWirepasSend((uint8_t *)&rpeMessage, sizeof(periodInfo_t) + (sizeof(nborInfo_t) * nborIdx), ASAFE_ENDPOINT_SRC_SR, ASAFE_ENDPOINT_PROXIMITY_V2_DST, APP_LIB_DATA_QOS_NORMAL);
        }

        scanPeriodMaxG = 0;
    }
}
#endif

void asProximityRequestScan(proximityScanReason_e scanRequestReason)
{
    static uint32_t lastScanRequestTime = 0;

    /*Prepare for the period - This will be used to define the period*/
    scanPeriodSeconds = lib_time->getTimestampS() - lastScanRequestTime;
    lastScanRequestTime = lib_time->getTimestampS();

    previousScanReason = currentScanReason;
    currentScanReason = scanRequestReason;

#ifdef AS_RTLS_MODE_HR
    switch (scanRequestReason)
    {
    case PROXIMITY_SCAN_REASON_TIME:
        asUWBTimerRequestStationaryScan();
        break;
    case PROXIMITY_SCAN_REASON_MOTION:
        asUWBTimerRequestMotionScan();
        break;
    }

#endif

#ifdef AS_RTLS_MODE_SR
    nborIdx = 0;
    Stack_State_addEventCb(scanend_cb, 1 << APP_LIB_STATE_STACK_EVENT_SCAN_STOPPED);
    lib_state->setOnBeaconCb(nborScanReceivedBeaconCB);
    lib_state->startScanNbors();
#endif
}

void asProximitySetMaxG(uint16_t maxG)
{
    scanPeriodMaxG = maxG;
}

// This function contains the main work for the tag
static uint32_t asProximityTagTask()
{
    static uint32_t lastUpdateTime = 0;
    static bool motionEndRequired = false; // This flag indicates that the tag has gone through some motion and thus we need to send a stationary to inform the system that the tag has stopped moving.
    // We only enable motion if the stationary is not the same as the motion update rate
    bool motionEnabled = !(asProximitySettings.updateRateMotionSeconds == asProximitySettings.updateRateSeconds);
    if (motionEnabled && motionDetected && (lib_time->getTimestampS() - lastUpdateTime) >= asProximitySettings.updateRateMotionSeconds)
    {
        lastUpdateTime = lib_time->getTimestampS();
        LOG(LVL_INFO, "Tag Position Update due to Motion\n");
        // Start a procedure to get a position update
        asProximityRequestScan(PROXIMITY_SCAN_REASON_MOTION);
        motionDetected = false;
        motionEndRequired = true;
    }
    else if (motionEnabled && motionEndRequired && (lib_time->getTimestampS() - lastUpdateTime) >= asProximitySettings.updateRateMotionSeconds)
    {
        lastUpdateTime = lib_time->getTimestampS();
        LOG(LVL_INFO, "Tag Position to Close Motion Period\n");
        // Start a procedure to get a position update
        asProximityRequestScan(PROXIMITY_SCAN_REASON_TIME);
        motionEndRequired = false;
    }
    else if ((lib_time->getTimestampS() - lastUpdateTime) >= asProximitySettings.updateRateSeconds)
    {
        lastUpdateTime = lib_time->getTimestampS();
        LOG(LVL_INFO, "Tag Position Update due to Time\n");
        // Start a procedure to get a position update
        asProximityRequestScan(PROXIMITY_SCAN_REASON_TIME);
    }
    return (TAG_TASK_DUTY_CYCLE_SECONDS * 1000);
}
#endif

#ifdef AS_RTLS_ROLE_ANCHOR
// This function contains the main work for the anchor
static uint32_t asProximityAnchorTask()
{
    // For now the anchor does not have any task (assuming its SR)
    return APP_SCHEDULER_STOP_TASK;
}
#endif

void asProximityInit()
{
    // This function shall only run if the node is provisioned.
    if (!Local_provisioning_is_provisioned())
    {
        LOG(LVL_INFO, "Node is not yet provisioned.  Positioning will not start until provisioning is complete.");
        return;
    }

#ifdef AS_RTLS_ROLE_ANCHOR
    LOG(LVL_INFO, "Starting Positioning Anchor Task");
    App_Scheduler_addTask_execTime(asProximityAnchorTask, APP_SCHEDULER_SCHEDULE_ASAP, 500);
#endif
#ifdef AS_RTLS_ROLE_TAG
    // Assume first scan reason was stationary
    previousScanReason = PROXIMITY_SCAN_REASON_TIME;

    // Clear the motion detected flag
    motionDetected = false;

    LOG(LVL_INFO, "Starting Tag Task");
    LOG(LVL_INFO, "Update Rate: %d seconds", asProximitySettings.updateRateSeconds);
    App_Scheduler_addTask_execTime(asProximityTagTask, APP_SCHEDULER_SCHEDULE_ASAP, 500);
    nborIdx = 0;
#endif
}

#ifdef AS_RTLS_ROLE_TAG
/*
This function is called to check whether a device is in motion.
We define a device being in motion if it experienced some kind of motion in the past 10 seconds
*/
bool asProximityIsDeviceInMotion()
{
    return ((lib_time->getTimestampS() - motionDetectedAtTimestamp) < MOTION_TIME_THRESHOLD_SECONDS || asLIS3DHInMotion());
}

/*
 * @brief This function is called by other modules to indicate that motion has been detected.  This will trigger a scan to be performed.
 */
void asProximityMotionDetected()
{
    motionDetected = true;
    motionDetectedAtTimestamp = lib_time->getTimestampS();
}
#endif

static uint32_t manufacturingDelayedReset()
{
    // We have to run a reset still
    lib_state->stopStack();
    return APP_SCHEDULER_STOP_TASK;
}

// This will be called from both NFC or Buttons
void asProximityToggleOperatingMode()
{
    // First Toggle the mode
    LED_FLASHES(0, 2);
    asProximitySettings.mode = asProximitySettings.mode == AS_MODE_INVENTORY ? AS_MODE_RUN : AS_MODE_INVENTORY;
    asStorageWriteProximitySettings();
    LOG(LVL_INFO, "Switched mode to %s", asProximitySettings.mode == AS_MODE_INVENTORY ? "INVENTORY" : "RUN");
    LOG_FLUSH(LVL_INFO);
    App_Scheduler_addTask_execTime(manufacturingDelayedReset, 2000, 500);
    return;
}