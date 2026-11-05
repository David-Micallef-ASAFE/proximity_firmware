/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_provisioning.h
 *
 */

#ifndef __AS_PROXIMITY_H_
#define __AS_PROXIMITY_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

/*Mode of the device*/
typedef enum
{
    AS_MODE_INVENTORY,
    AS_MODE_RUN
} asMode_e;

/*Structure to hold settings for the proximity part of the appolication*/
typedef struct
{
    uint32_t pskId; // Cannot be stored together with the regular keys so we store it here
    uint8_t pskKey[32];
    uint32_t heartbeatPeriodSeconds;
    asMode_e mode;
    bool deviceManagementEnabled;
    bool startInManufacturing;
#ifdef APP_UWB
    bool uwbOperationEnabled;
#endif
#ifdef AS_RTLS_ROLE_ANCHOR
    uint32_t rstSlotOffset; // By default this is set to 0 and then random is used but we can remotely control this
#endif
#ifdef AS_RTLS_ROLE_TAG
    uint32_t updateRateSeconds;       // How often the tag will send its position to the anchor
    uint32_t updateRateMotionSeconds; // How often the tag will send its position to the anchor
#endif
} asProximitySettings_t;

typedef enum
{
    PROXIMITY_SCAN_REASON_TIME = 0, // Stationary
    PROXIMITY_SCAN_REASON_MOTION = 1,
} proximityScanReason_e;

void asProximityInit();
void asProximityMotionDetected();
bool asProximityIsDeviceInMotion();

void asProximityToggleOperatingMode();

#endif /* __AS_PROXIMITY_H_ */