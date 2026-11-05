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
#include "as_protocol.h"
#include "as_storage.h"
#include "as_proximity.h"

#define DEBUG_LOG_MODULE_NAME "AS STATUS"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#define DESTINATION_ENDPOINT_CONFIG AS_PROTOCOL_ENDPOINT_REQUEST_DST

#include "debug_log.h"

static uint8_t protocolResponse[AS_PROTOCOL_MAX_MSG_SIZE];

extern asProximitySettings_t asProximitySettings;

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

bool asWirepasSinkRouteEstablished()
{
    app_lib_state_route_info_t routeInfo;
    lib_state->getRouteInfo(&routeInfo);
    return (routeInfo.state == APP_LIB_STATE_ROUTE_STATE_VALID);
}

/**
 * \brief   Data reception callback
 * \param   data
 *          Received data, \ref app_lib_data_received_t
 * \return  Result code, \ref app_lib_data_receive_res_e
 */
static app_lib_data_receive_res_e dataReceivedCb(
    const shared_data_item_t *item,
    const app_lib_data_received_t *data)
{
    (void)item;

    LOG(LVL_INFO, "Data Received");

    // Process the data.  data->bytes contains our payload
    LOG_BUFFER(LVL_INFO, data->bytes, data->num_bytes);

    // Decode the CBOR request and reply (an empty or invalid payload gets a MALFORMED reply)
    size_t responseLen = asProtocolProcessRequest(data->bytes, data->num_bytes, protocolResponse, sizeof(protocolResponse));
    if (responseLen > 0)
    {
        asWirepasSend(protocolResponse, (uint8_t)responseLen, AS_PROTOCOL_ENDPOINT_RESPONSE_SRC, AS_PROTOCOL_ENDPOINT_RESPONSE_DST, APP_LIB_DATA_QOS_NORMAL);
    }

    // Data handled successfully
    return APP_LIB_DATA_RECEIVE_RES_HANDLED;
}

/*Unicast messages filter*/
static shared_data_item_t alltype_packets_filter =
    {
        .cb = dataReceivedCb,
        .filter = {
            .mode = SHARED_DATA_NET_MODE_ALL,
            /* Filtering by source endpoint. */
            .src_endpoint = -1,
            /* Filtering by destination endpoint. */
            .dest_endpoint = DESTINATION_ENDPOINT_CONFIG,
            .multicast_cb = NULL}};

bool requireResetSettings;

/**
 * \brief   This function initializes the wirepas connector
 */
void asWirepasInit()
{
    // Set callback for received unicast messages
    Shared_Data_addDataReceivedCb(&alltype_packets_filter);

    requireResetSettings = false;
}

static void on_stack_event_cb(app_lib_stack_event_e event, void *param)
{
    switch (event)
    {
    case APP_LIB_STATE_STACK_EVENT_STACK_STOPPED:
        if (requireResetSettings)
        {
            lib_settings->resetAll();
            asProximitySettings.deviceManagementEnabled = true; // We enable this to ensure we do not leave a device in inventory and without a way to get out.
            asStorageWriteProximitySettings();                  // Ensure the settings are reinit into storage
        }
        break;
    default:
        break;
    }
}

static uint32_t resetWirepasSettingsTask()
{
    // Note the reset All option is not available when stack is running
    LOG(LVL_INFO, "Reset WP Settings - Unprovisioning");
    LOG_FLUSH(LVL_INFO);
    requireResetSettings = true;
    lib_state->setOnStackEventCb(on_stack_event_cb);
    lib_state->stopStack();
    return APP_SCHEDULER_STOP_TASK;
}

#define WIREPAS_RESET_DELAY_SECONDS 60

void asWirepasResetSettings()
{

    App_Scheduler_addTask_execTime(resetWirepasSettingsTask, (WIREPAS_RESET_DELAY_SECONDS * 1000), 500);
}