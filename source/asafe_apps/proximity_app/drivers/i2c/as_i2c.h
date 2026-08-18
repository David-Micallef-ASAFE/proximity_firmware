/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_i2c.h
 *
 */

#ifndef __AS_I2C_H_
#define __AS_I2C_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

#include "i2c.h"

void asI2CHWClose();
i2c_res_e asI2CHWOpen();

#endif /* __AS_PROVISIONING_H_ */