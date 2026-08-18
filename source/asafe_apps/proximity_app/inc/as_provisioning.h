/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_provisioning.h
 *
 */

#ifndef __AS_PROVISIONING_H_
#define __AS_PROVISIONING_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

void asProvisioningInit();
void asProvisioningStart();

void asProvisioningOnProvisionProxyEnabledCB(bool enabled);

#endif /* __AS_PROVISIONING_H_ */