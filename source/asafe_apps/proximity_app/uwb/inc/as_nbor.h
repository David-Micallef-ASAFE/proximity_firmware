/* Copyright 2022 Redlore. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

#ifndef __UWB_NBOR_H_
#define __UWB_NBOR_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"

#define UWB_NBOR_RS_WORKING_AREA_TICK 743

typedef struct
{
    bool isFree;
    uint32_t address;
    uint32_t lastUpdate;
    uint8_t assignedSlot;
    uint8_t slotOffset;
    uint8_t nbors;
    uint8_t rssi;
    bool isBestNbor; // Used for stationary scanning
} uwbNborSlot_t;

void nborManagerInitStart();
void nborManagerInitEnd();
// void nborManagerAddNbor(uint32_t address, uint32_t remoteRSTimestamp);
void nborManagerAddNbor(uint32_t address, uint32_t remoteRSTimestamp, uint8_t losQuality);
void nborManagerProcess();
bool nborManagerIsProcessingRequired();
void nborManagerScanDone();
void nborManagerAddNborThroughRST(uint32_t address, uint32_t remoteRSTimestamp);

void nborManagerInit();
// void nborManagerAddNborThroughUWB(uint32_t address, uint32_t nextRSUs, uint32_t processingDelay, uint8_t nborsNum);
void nborManagerAddNborThroughUWB(uint32_t address, uint32_t nextRSUs, uint32_t processingDelay, uint8_t nborsNum, uint8_t rssi);
void nborManagerPrintSlots();
uwbNborSlot_t *nborManagerGetUWBSlot(uint8_t idx);
uwbNborSlot_t *nborManagerRSAvailable(uint32_t timerSlot);
void nborManagerProcessForBestAnchor();

bool nborManagerNborsAvailable();

uwbNborSlot_t *nborManagerRequireNborRST(uint32_t timerSlot);
uint8_t nborManagerGetOffset();
uint8_t nborManagerGetAvailableAnchors();

#endif //__UWB_NBOR_H_