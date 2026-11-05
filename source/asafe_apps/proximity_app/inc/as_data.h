/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_provisioning.h
 *
 */

#ifndef __AS_DATA_H_
#define __AS_DATA_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

#define NBOR_SCAN_MAX_NBORS 8

/*RPE - Low Res Nbor*/
typedef struct __attribute__((packed))
{
    uint32_t address;
    int8_t normRSSI;
    uint8_t linkReliability;
    int8_t txPower;
    int8_t rxPower;
    uint16_t lastUpdate;
} nborInfo_t;

/*RPE - Low Res - Header*/
typedef struct __attribute__((packed))
{
    uint8_t sequence;
    uint8_t type; // Type of the period (Stationary = 0, Motion = 1)
    uint32_t duration;
    uint16_t maxG; // mG
    uint16_t battery;
} periodInfo_t;

typedef struct __attribute__((packed))
{
    periodInfo_t periodHeader;
    nborInfo_t nbors[NBOR_SCAN_MAX_NBORS];
} rpeMessage_t;

#endif /* __AS_DATA_H_ */
