/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_types.h
 *
 */

#ifndef __AS_TYPES_H_
#define __AS_TYPES_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

#define DEVICE_TYPE_MINEW_MBT02 30                       // Find it Tag 3 - Asset Tag SR with Retrieval
#define DEVICE_TYPE_MINEW_C10 31                         // Find it Tag 3 - Asset Tag SR with Retrieval
#define DEVICE_TYPE_MINEW_E9 32                          // Find it Tag 3 - Asset Tag SR with Retrieval
#define DEVICE_TYPE_KKM_K4W 33                           // Find it Tag 3 - Asset Tag SR with Retrieval
#define DEVICE_TYPE_KKM_C3W 34                           // Find it Tag 3 - Asset Tag SR with Retrieval
#define DEVICE_TYPE_ASAFE_JIG 35                         // ASAFE JIG
#define DEVICE_TYPE_ASAFE_ANCHOR_HR 36                   // ASAFE Anchor HR
#define DEVICE_TYPE_ASAFE_ASSET_TAG_HR_WITH_RETRIEVAL 37 // ASAFE Asset Tag HR with Retrieval
#define DEVICE_TYPE_ASAFE_ASSET_TAG_SR_LARGE 38          // ASAFE Asset Tag SR Large

/*Source Endpoint*/
#define ASAFE_ENDPOINT_SRC_SR 10
#define ASAFE_ENDPOINT_SRC_HR_TAG 31
#define ASAFE_ENDPOINT_SRC_HR_ANCHOR 30
#define ASAFE_ENDPOINT_SRC_SR_RPE 238 // This was initially used for the RPE data from the SR tag for compatibility with the WPE.

/*Destination Endpoint*/
#define ASAFE_ENDPOINT_STATUS_DST 10
#define ASAFE_ENDPOINT_PROXIMITY_STATIONARY_DST 239
#define ASAFE_ENDPOINT_PROXIMITY_MOTION_DST 238
#define ASAFE_ENDPOINT_PROXIMITY_V2_DST 16
#define ASAFE_ENDPOINT_UWB_TIMING_DST 30

/*Device Role*/
typedef enum
{
    DEVICE_ROLE_RESERVED = 0,
    DEVICE_ROLE_TAG = 1,
    DEVICE_ROLE_ANCHOR = 2,
    DEVICE_ROLE_PROXIMITY_ANCHOR = 3
} deviceRole_e;

/*Status Message*/
typedef struct __attribute__((packed))
{
    uint16_t deviceType;
    uint16_t deviceRole;
    uint16_t sequence;
    uint16_t vbat;
    uint8_t wirepasRole;
    uint32_t softwareVersion;
    uint32_t wirepasStackVersion;
    uint32_t appAreaId;
} statusMsg_t;

#endif /* __AS_TYPES_H_ */