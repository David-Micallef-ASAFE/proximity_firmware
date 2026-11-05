/* Copyright 2022 Redlore. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

#include "as_uwbTimer.h"
#include "board.h"

#include "as_uwb.h"

#include "as_wirepas.h"
#include "random.h"
#include "as_types.h"

/*----------------DEBUG-------------------------------------------*/
#define DEBUG_LOG_MODULE_NAME "RANGING"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO
#include "debug_log.h"
/*----------------DEBUG-------------------------------------------*/

#define ANCHORS_NUM_MAX 20

typedef struct __attribute__((packed))
{
    uint32_t address;
    uint32_t rst;
    uint8_t rssi;
    uint8_t losQuality;
} anchor_t;

typedef struct __attribute__((packed))
{
    uint8_t messageIdxIndicator;
    uint8_t sequence;
    uint32_t rss;
    uint32_t rse;
    anchor_t anchors[ANCHORS_NUM_MAX];
    uint8_t anchorIdx;
} rangingSlot_t;

rangingSlot_t rangingSlot;

bool rssReceived;

bool commsReceived; // This is high if at least a device has communicated with the system.

#define STATIC_MESSAGE_SIZE_BYTES 10
#define FRAME_ANCHORS_NUM 9

bool inMotion;
uint32_t lastRangingSentTimestamp;

uint32_t rangingSlotReady()
{
    // uint8_t anchorIdx;
    uint8_t framesRequired;
    uint8_t frameIdx;
    uint8_t anchorsTransmitted;

    LOG(LVL_INFO, "RSSE %u %u %d", rangingSlot.rss, rangingSlot.rse, rangingSlot.anchorIdx);
    // for (anchorIdx = 0; anchorIdx < rangingSlot.anchorIdx; anchorIdx++)
    // {
    //     // LOG(LVL_INFO, "Anchor %u %u", rangingSlot.anchors[anchorIdx].address, rangingSlot.anchors[anchorIdx].rst);
    // }
    // Transmit the message
    framesRequired = (rangingSlot.anchorIdx / FRAME_ANCHORS_NUM);
    framesRequired += (rangingSlot.anchorIdx % FRAME_ANCHORS_NUM) > 0 ? 1 : 0;

    // Check in the WP Comm system if we have enough space for this message

    for (frameIdx = 0; frameIdx < framesRequired; frameIdx++)
    {
        // We set the message indicator
        // Bits 0 to 3 indicate the amount of frames in a message,
        // Bits 4 to 7 indicate which frame is being sent
        // E.g [00100100] Bits 0 to 3 tell us this message is made up of 4 frames (of 100 bytes each) and Bits 4 to 7 tell us that this is the 3rd frame (as we start from 0)
        rangingSlot.messageIdxIndicator = (frameIdx << 4) + framesRequired;
        anchorsTransmitted = frameIdx == (framesRequired - 1) ? rangingSlot.anchorIdx - (frameIdx * FRAME_ANCHORS_NUM) : FRAME_ANCHORS_NUM;
        if (frameIdx > 0)
        { // First we need to shift the anchors
            memcpy(&rangingSlot.anchors[0], &rangingSlot.anchors[FRAME_ANCHORS_NUM * frameIdx], sizeof(anchor_t) * anchorsTransmitted);
        }
        // LOG(LVL_INFO, "F %u, A %u, %u", framesRequired, anchorsTransmitted, rangingSlot.anchorIdx);
        //  Motion -
        // Bug needs to be on the first message only
#ifdef AS_RTLS_ROLE_TAG
        if (frameIdx == 0)
        { // We only do this in the first fram when we are sending the serving anchor
            // rangingSlot.anchors[0].rst = motionPopAccumulator();
            if (inMotion)
            {
                rangingSlot.anchors[0].rst = 0X00008000; // Motion Indicator
                rangingSlot.anchors[0].rst += lib_time->getTimestampS() - lastRangingSentTimestamp;
            }
            else
            {
                rangingSlot.anchors[0].rst = 0;
            }
            lastRangingSentTimestamp = lib_time->getTimestampS();
#ifdef APP_MECHANICAL
            uint32_t motionMaxPeriodTemp;
            if (rangingSlot.anchors[0].rst != 0)
            {
                motionMaxPeriodTemp = motionMaxPeriod();
                motionMaxPeriodTemp <<= 16;
                rangingSlot.anchors[0].rst += motionMaxPeriodTemp;
            }
            motionMaxPeriodClear();
#endif
        }
#endif

// LOG(LVL_INFO, "M %u", rangingSlot.anchors[0].rst);
#ifdef AS_RTLS_ROLE_TAG
        asWirepasSend((uint8_t *)&rangingSlot, STATIC_MESSAGE_SIZE_BYTES + (sizeof(anchor_t) * anchorsTransmitted), ASAFE_ENDPOINT_SRC_HR_TAG, ASAFE_ENDPOINT_UWB_TIMING_DST, APP_LIB_DATA_QOS_NORMAL);
#endif
#ifdef AS_RTLS_ROLE_ANCHOR
        asWirepasSend((uint8_t *)&rangingSlot, STATIC_MESSAGE_SIZE_BYTES + (sizeof(anchor_t) * anchorsTransmitted), ASAFE_ENDPOINT_SRC_HR_ANCHOR, ASAFE_ENDPOINT_UWB_TIMING_DST, APP_LIB_DATA_QOS_NORMAL);
#endif
    }

    //  Prepare for the next slot
    rangingSlot.rss = 0;
    rangingSlot.rse = 0;
    rangingSlot.sequence++;
    rangingSlot.anchorIdx = 0;
    rssReceived = false;

    return APP_SCHEDULER_STOP_TASK;
}

uint8_t rangingSlotSequence()
{
    return rangingSlot.sequence;
}

void rangingAddAnchor(uint32_t address, uint32_t rst, uint8_t rssi, uint8_t losQuality)
{
    commsReceived = true;
    if (rssReceived)
    {
        if (rangingSlot.anchorIdx < ANCHORS_NUM_MAX)
        {
            rangingSlot.anchors[rangingSlot.anchorIdx].address = address;
            rangingSlot.anchors[rangingSlot.anchorIdx].rst = rst;
            rangingSlot.anchors[rangingSlot.anchorIdx].rssi = rssi;
            rangingSlot.anchors[rangingSlot.anchorIdx].losQuality = losQuality;
            rangingSlot.anchorIdx++;
        }
    }
}

void rangingSlotRSS(uint32_t address, uint32_t rss, uint8_t sequence, uint8_t rssi, uint8_t losQuality)
{
    commsReceived = true;
    rangingSlot.rss = rss;
    rssReceived = true;
#ifdef AS_RTLS_ROLE_TAG
    rangingSlot.sequence = sequence;
    rangingAddAnchor(address, 0, rssi, losQuality);
#endif
}

void rangingSlotRSE(uint32_t rse)
{
    commsReceived = true;
    rangingSlot.rse = rse;
}

void rangingInit()
{
    rangingSlot.sequence = 0;
    rangingSlot.anchorIdx = 0;
    rssReceived = false;
    lastRangingSentTimestamp = 0;
    inMotion = false;
    commsReceived = false;
}

void rangingStart(bool motionCondition)
{
    inMotion = motionCondition;

    // This is to ensure that we start clean
    rangingSlot.rss = 0;
    rangingSlot.rse = 0;
    rangingSlot.anchorIdx = 0;

    rssReceived = false;

    commsReceived = false;
}

bool rangingCommsReceived()
{
    return commsReceived;
}