/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_lis3dh.h
 *
 */

#ifndef __AS_LIS3DH_H_
#define __AS_LIS3DH_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

#include "i2c.h"

void asLIS3DHInit();
bool asLIS3DHEnableMotionDetect();
bool asLIS3DHInMotion();

#endif /* #define __AS_LIS3DH_H_ */