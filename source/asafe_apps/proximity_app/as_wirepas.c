/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_wirepas.c
 * \brief   This file handles all the wirepas related logic
 */

#include <stdlib.h>

#include "api.h"
#include "node_configuration.h"
#include "shared_data.h"
#include "app_scheduler.h"

#include "as_types.h"

#define DEBUG_LOG_MODULE_NAME "AS STATUS"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

app_lib_data_send_res_e asWirepasSend(uint8_t *buffer, uint8_t size, uint8_t srcEndpoint, uint8_t destEndpoint, app_lib_data_qos_e priority)
{
    LOG(LVL_INFO, "Sending Data");

    // Create a data packet to send
    app_lib_data_to_send_t data_to_send;
    data_to_send.bytes = (const uint8_t *)buffer;
    data_to_send.num_bytes = size;
    data_to_send.dest_address = APP_ADDR_ANYSINK;
    data_to_send.src_endpoint = srcEndpoint;
    data_to_send.dest_endpoint = destEndpoint;
    data_to_send.qos = priority;
    data_to_send.flags = APP_LIB_DATA_SEND_FLAG_NONE;
    data_to_send.tracking_id = APP_LIB_DATA_NO_TRACKING_ID;

    // Send the data packet
    return Shared_Data_sendData(&data_to_send, NULL);
}

/**
 * \brief   This function initializes the wirepas connector
 */
void asWirepasInit()
{
}
