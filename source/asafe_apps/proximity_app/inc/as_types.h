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

#define DEVICE_TYPE_MBT02 7 // Find it Tag 3 - Asset Tag SR with Retrieval

/*Source Endpoint*/
#define ASAFE_ENDPOINT_SRC_SR 10

/*Destination Endpoint*/
#define ASAFE_ENDPOINT_STATUS_DST 10

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