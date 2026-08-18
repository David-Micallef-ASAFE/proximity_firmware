/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_wirepas.h
 *
 */

#ifndef __AS_WIREPAS_H_
#define __AS_WIREPAS_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

void asWirepasInit();
app_lib_data_send_res_e asWirepasSend(uint8_t *buffer, uint8_t size, uint8_t srcEndpoint, uint8_t destEndpoint, app_lib_data_qos_e priority);

#endif /* __AS_WIREPAS_H_ */