/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_protocol.h
 * \brief Device <-> Cloud CBOR protocol (see claude/cbor-device-cloud-protocol.md)
 */

#ifndef __AS_PROTOCOL_H_
#define __AS_PROTOCOL_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Protocol version supported by this device (§4.3) */
#define AS_PROTOCOL_VERSION 1

/* Maximum message size in either direction (§2) */
#define AS_PROTOCOL_MAX_MSG_SIZE 100

/* Wirepas endpoints (§9.1) */
#define AS_PROTOCOL_ENDPOINT_REQUEST_DST 10
#define AS_PROTOCOL_ENDPOINT_RESPONSE_SRC 10
#define AS_PROTOCOL_ENDPOINT_RESPONSE_DST 2

/* Control field keys (§4) */
#define AS_PROTO_CTRL_OPERATION (-1)
#define AS_PROTO_CTRL_SEQUENCE (-2)
#define AS_PROTO_CTRL_STATUS (-3)
#define AS_PROTO_CTRL_KEY_LIST (-4)
#define AS_PROTO_CTRL_KEY_ERRORS (-5)
#define AS_PROTO_CTRL_VERSION (-6)

/* Operations (§4.1) */
typedef enum
{
    AS_PROTO_OP_READ = 1,
    AS_PROTO_OP_WRITE = 2,
    AS_PROTO_OP_RESPONSE = 3,
    AS_PROTO_OP_NOTIFY = 4
} asProtoOp_e;

/* Status codes (§5) */
typedef enum
{
    AS_PROTO_STATUS_OK = 0,
    AS_PROTO_STATUS_UNKNOWN_KEY = 1,
    AS_PROTO_STATUS_ACCESS_DENIED = 2,
    AS_PROTO_STATUS_INVALID_TYPE = 3,
    AS_PROTO_STATUS_OUT_OF_RANGE = 4,
    AS_PROTO_STATUS_MSG_TOO_LARGE = 5,
    AS_PROTO_STATUS_MALFORMED = 6,
    AS_PROTO_STATUS_UNSUPPORTED_OP = 7,
    AS_PROTO_STATUS_BUSY = 8,
    AS_PROTO_STATUS_PARTIAL = 9,
    AS_PROTO_STATUS_UNSUPPORTED_VERSION = 10
} asProtoStatus_e;

/* Data keys (§6) */
typedef enum
{
    AS_PROTO_KEY_NETWORK_ADDRESS = 1,
    AS_PROTO_KEY_NETWORK_CHANNEL = 2,
    AS_PROTO_KEY_NODE_ROLE = 3,
    AS_PROTO_KEY_NETWORK_KEYS_IN_USE = 4,
    AS_PROTO_KEY_NETWORK_CIPHER_KEY = 5,
    AS_PROTO_KEY_NETWORK_AUTH_KEY = 6,
    AS_PROTO_KEY_PROVISIONING_UID = 7,
    AS_PROTO_KEY_PROVISIONING_KEY = 8,
    AS_PROTO_KEY_HEARTBEAT = 9,
    AS_PROTO_KEY_ANCHOR_SLOT = 10,
    AS_PROTO_KEY_TAG_STATIONARY_PERIOD = 11,
    AS_PROTO_KEY_TAG_MOTION_PERIOD = 12,
    AS_PROTO_KEY_RESET_DEVICE_SETTINGS = 13,
    AS_PROTO_KEY_RETRIEVE = 14,
    AS_PROTO_KEY_MODE = 15,
    AS_PROTO_KEY_PING = 16,
    AS_PROTO_KEY_UWB_OPERATION_ENABLED = 17,
    AS_PROTO_KEY_DEVICE_MANAGEMENT_ENABLED = 18
} asProtoKey_e;

/* Value type of a data key, as defined in the Key Database */
typedef enum
{
    AS_PROTO_TYPE_UINT,
    AS_PROTO_TYPE_BOOL,
    AS_PROTO_TYPE_BYTES, /* Fixed length, given by maxValue in the key table */
    AS_PROTO_TYPE_NULL   /* Command keys without a parameter */
} asProtoType_e;

/* Value passed to / from the key handlers */
typedef struct
{
    uint32_t u32;         /* AS_PROTO_TYPE_UINT */
    bool boolean;         /* AS_PROTO_TYPE_BOOL */
    const uint8_t *bytes; /* AS_PROTO_TYPE_BYTES. On write this points into the received
                             packet and is only valid during the handler call: copy it. */
    uint8_t numBytes;
} asProtoValue_t;

/**
 * \brief   Read handler. Fills \p value with the current value of the key.
 * \return  AS_PROTO_STATUS_OK, or an error status reported for that key
 */
typedef asProtoStatus_e (*asProtoReadCb_f)(asProtoValue_t *value);

/**
 * \brief   Write handler. Called only once every key of the WRITE has passed
 *          type and range validation (all-or-nothing, §4.2).
 * \return  AS_PROTO_STATUS_OK, or an error status reported for that key
 */
typedef asProtoStatus_e (*asProtoWriteCb_f)(const asProtoValue_t *value);

/**
 * \brief   Optional extra validation beyond the min/max range check.
 *          Must not change anything on the device.
 */
typedef asProtoStatus_e (*asProtoValidateCb_f)(const asProtoValue_t *value);

/* One entry of the Key Database. A NULL read (write) callback makes the key write-only (read-only). */
typedef struct
{
    uint8_t key;
    asProtoType_e type;
    uint32_t minValue; /* UINT: minimum value */
    uint32_t maxValue; /* UINT: maximum value. BYTES: exact length */
    asProtoReadCb_f read;
    asProtoWriteCb_f write;
    asProtoValidateCb_f validate;
    const char *name;
} asProtoKeyInfo_t;

/**
 * \brief   Decodes a request and builds the response
 * \param   request      Received payload
 * \param   requestLen   Payload length
 * \param   response     Buffer for the response, at least AS_PROTOCOL_MAX_MSG_SIZE bytes
 * \param   responseSize Size of the response buffer
 * \return  Length of the response to send, 0 if nothing must be sent
 */
size_t asProtocolProcessRequest(const uint8_t *request, size_t requestLen, uint8_t *response, size_t responseSize);

#endif /* __AS_PROTOCOL_H_ */
