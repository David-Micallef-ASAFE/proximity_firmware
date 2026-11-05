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

#include "as_nbor.h"

/*----------------DEBUG-------------------------------------------*/
#define DEBUG_LOG_MODULE_NAME "NBOR MGR"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO
#include "debug_log.h"
/*----------------DEBUG-------------------------------------------*/

#define UWB_NBORS_MAX 20

// 1 Tick is 30uS
#define UWB_NBOR_RS_START_DELAY_TICK 26

#define UWB_NBOR_MAX_LAST_UPDATE_SEC_TO_FREE (30 * 60) // 30 Minutes

uwbNborSlot_t uwbNborSlots[UWB_NBORS_MAX];
uint8_t nborAnchorsNumDynamic;

// This function is called once at the start of the program to initialize the lastUpdateReceivedAt of the slot
void nborManagerInit()
{
    uint8_t nborSlotIdx;
    for (nborSlotIdx = 0; nborSlotIdx < UWB_NBORS_MAX; nborSlotIdx++)
    {
        uwbNborSlots[nborSlotIdx].isFree = true;
    }
    nborAnchorsNumDynamic = 0;
}

void nborManagerAddNborThroughUWB(uint32_t address, uint32_t nextRSUs, uint32_t processingDelay, uint8_t nborsNum, uint8_t rssi)
{
    uint8_t nborSlotIdx, freeSlotIdx;
    bool freeSlotFound = false;

    uint32_t currentTimestamp = lib_time->getTimestampS();
    int16_t assignedSlotIdxRes;

    uint32_t nextRSSlots;
    uint32_t nextRSSlotOffsetMs;

    uint8_t nborsAvailable = 0;

    // LOG(LVL_INFO, "A %u", address);

    // we need to find in which slot to put the nbor and by what offset in that slot
    // Check if we are already in the list
    for (nborSlotIdx = 0; nborSlotIdx < UWB_NBORS_MAX; nborSlotIdx++)
    {
        if (uwbNborSlots[nborSlotIdx].address == address)
        {
            uwbNborSlots[nborSlotIdx].isFree = true;
        }
        if (!uwbNborSlots[nborSlotIdx].isFree)
        {
            nborsAvailable++;
        }
    }

    for (nborSlotIdx = 0; nborSlotIdx < UWB_NBORS_MAX && !freeSlotFound; nborSlotIdx++)
    {
        if (uwbNborSlots[nborSlotIdx].isFree)
        {
            freeSlotIdx = nborSlotIdx;
            freeSlotFound = true;
        }
    }

    if (!freeSlotFound)
    { // We are full up
        nborAnchorsNumDynamic = nborsAvailable;
        return;
    }
    nborsAvailable++;
    nborAnchorsNumDynamic = nborsAvailable;
    // If we are here it means we are good to go
    //  If we get here it means we found a slot
    uwbNborSlots[freeSlotIdx].lastUpdate = currentTimestamp;
    // To calculate the offset we need the time that has passed from the timer - for now ignoring the processing delay
    // We know, in uS, the time the RS will occur.  We now need to offset this from our own time.
    nextRSUs += timerGetSlotOffsetUs(); // Align with our slot timing.
    nextRSSlots = nextRSUs / TIMER_SLOT_US;
    nextRSSlotOffsetMs = (nextRSUs % TIMER_SLOT_US) / 1000;
    uwbNborSlots[freeSlotIdx].slotOffset = nextRSSlotOffsetMs;
    uwbNborSlots[freeSlotIdx].address = address;
    uwbNborSlots[freeSlotIdx].nbors = nborsNum;
    uwbNborSlots[freeSlotIdx].rssi = rssi;

    // Inform timer about the nbor
    assignedSlotIdxRes = timerGetSlotForNbor(nextRSSlots, freeSlotIdx);
    if (assignedSlotIdxRes < 0)
    {
        // Slot was not assigned
        uwbNborSlots[freeSlotIdx].isFree = true;
    }
    else
    {
        uwbNborSlots[freeSlotIdx].assignedSlot = assignedSlotIdxRes;
        uwbNborSlots[freeSlotIdx].isFree = false;
    }
}

void nborManagerPrintSlots()
{
    uint8_t nborSlotIdx;
#ifdef APP_PRINTING
    uint32_t currentTimestamp = lib_time->getTimestampS();
#endif

    for (nborSlotIdx = 0; nborSlotIdx < 10; nborSlotIdx++)
    {
        if (!uwbNborSlots[nborSlotIdx].isFree)
        {
            LOG(LVL_INFO, "Nbord %u %d %d", uwbNborSlots[nborSlotIdx].address, uwbNborSlots[nborSlotIdx].assignedSlot, (currentTimestamp - uwbNborSlots[nborSlotIdx].lastUpdate));
            LOG_FLUSH(LVL_INFO);
        }
    }
}

uwbNborSlot_t *nborManagerGetUWBSlot(uint8_t idx)
{
    return &uwbNborSlots[idx];
}

uwbNborSlot_t *nborManagerRSAvailable(uint32_t timerSlot)
{
    uint8_t nborSlotIdx;
    for (nborSlotIdx = 0; nborSlotIdx < UWB_NBORS_MAX; nborSlotIdx++)
    {
        if (!uwbNborSlots[nborSlotIdx].isFree && (uwbNborSlots[nborSlotIdx].assignedSlot == timerSlot))
        {
            return &uwbNborSlots[nborSlotIdx];
        }
    }
    return NULL;
}

uint8_t nborManagerGetOffset()
{
    uint8_t nborSlotIdx;
    uint8_t offset = 0; // Number of neighbours with a lower address than ours
    uint32_t deviceAddress;
    lib_settings->getNodeAddress(&deviceAddress);

    for (nborSlotIdx = 0; nborSlotIdx < UWB_NBORS_MAX; nborSlotIdx++)
    {
        if (uwbNborSlots[nborSlotIdx].isFree)
        {
            continue; // Empty slot, ignore
        }

        if (uwbNborSlots[nborSlotIdx].address < deviceAddress)
        {
            offset++;
        }
    }
    return offset;
}

void nborManagerProcessForBestAnchor()
{
    // Find the Closest anchor with more than 5Nbors
    uint8_t nborLimit = 5;
    uint8_t nborSlotIdx, bestSlotIdx;
    uint8_t bestRSSI = 0;
    bool bestSlotFound = false;
    uint32_t currentTimestamp = lib_time->getTimestampS();

    do
    {
        for (nborSlotIdx = 0; nborSlotIdx < UWB_NBORS_MAX; nborSlotIdx++)
        {
            // Expire bad nbors
            if (uwbNborSlots[nborSlotIdx].nbors < 3 || (currentTimestamp - uwbNborSlots[nborSlotIdx].lastUpdate) > 60)
            {
                uwbNborSlots[nborSlotIdx].isFree = true;
            }
            if (!uwbNborSlots[nborSlotIdx].isFree && uwbNborSlots[nborSlotIdx].nbors >= nborLimit)
            {
                if (uwbNborSlots[nborSlotIdx].rssi > bestRSSI)
                {
                    bestSlotIdx = nborSlotIdx;
                    bestRSSI = uwbNborSlots[nborSlotIdx].rssi;
                    bestSlotFound = true;
                }
            }
            uwbNborSlots[nborSlotIdx].isBestNbor = false;
        }
        // Decrease the nborLimit in case we did not find a good slot
        nborLimit--;
    } while (!bestSlotFound && nborLimit > 2);

    if (bestSlotFound)
    {
        LOG(LVL_INFO, "BS %d", bestSlotIdx);
        uwbNborSlots[bestSlotIdx].isBestNbor = true;
    }
}

bool nborManagerNborsAvailable()
{
    uint8_t nborSlotIdx;
    uint32_t currentTimestamp = lib_time->getTimestampS();
    for (nborSlotIdx = 0; nborSlotIdx < UWB_NBORS_MAX; nborSlotIdx++)
    {
        if (!uwbNborSlots[nborSlotIdx].isFree && (currentTimestamp - uwbNborSlots[nborSlotIdx].lastUpdate) < 300)
        {
            // If we get here it means that we have at least one nbor and we can run on its slot
            return true;
        }
    }
    // If we get here it means we do not have any nbors
    return false;
}

uwbNborSlot_t *nborManagerRequireNborRST(uint32_t timerSlot)
{
    uint8_t nborSlotIdx;
    uint32_t currentTimestamp = lib_time->getTimestampS();

    for (nborSlotIdx = 0; nborSlotIdx < UWB_NBORS_MAX; nborSlotIdx++)
    {
        // Add expiration
        if (!uwbNborSlots[nborSlotIdx].isFree && (currentTimestamp - uwbNborSlots[nborSlotIdx].lastUpdate) > 600)
        {
            uwbNborSlots[nborSlotIdx].isFree = true;
            LOG(LVL_INFO, "Nbor %u Expired", uwbNborSlots[nborSlotIdx].address);
        }
        // Check if within this slot
        if (!uwbNborSlots[nborSlotIdx].isFree && (uwbNborSlots[nborSlotIdx].assignedSlot == timerSlot))
        {
            return &uwbNborSlots[nborSlotIdx];
        }
    }
    return NULL;
}

uint8_t nborManagerGetAvailableAnchors()
{
    return nborAnchorsNumDynamic;
}