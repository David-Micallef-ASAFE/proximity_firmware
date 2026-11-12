/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_protocol.c
 * \brief   Device <-> Cloud CBOR protocol (see claude/cbor-device-cloud-protocol.md).
 *          Decodes READ/WRITE requests, validates them and builds the RESPONSE.
 *
 *          The first section holds the per-key handlers and the Key Database (§6).
 *          The engine checks the CBOR type and min/max range of a value before calling a
 *          write handler, and only calls the write handlers once every key of the WRITE is
 *          valid (all-or-nothing). The handlers are templates: they log the request and
 *          return OK but do not act on the device yet. Add device code where marked TODO.
 *          Read handlers return placeholder values until implemented.
 */

#include <string.h>

#include "as_cbor.h"
#include "as_protocol.h"
#include "as_proximity.h"
#include "as_storage.h"
#include "as_provisioning.h"
#include "as_wirepas.h"
#include "as_buzzer.h"

#define DEBUG_LOG_MODULE_NAME "AS PROTOCOL"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

extern asProximitySettings_t asProximitySettings;

#define MAX_RETRIEVAL_TIME_SECONDS 3600

/******************************************************************************
 * Key handlers (templates)
 *****************************************************************************/

/* Key 1 - Network Address (R/W, Wirepas) */
static asProtoStatus_e readNetworkAddress(asProtoValue_t *value)
{
    LOG(LVL_INFO, "READ Network Address");
    // TODO: Return the current Wirepas network address
    uint32_t networkAddress;
    lib_settings->getNetworkAddress(&networkAddress);
    value->u32 = networkAddress;
    return AS_PROTO_STATUS_OK;
}

static asProtoStatus_e writeNetworkAddress(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Network Address: 0x%06x", (unsigned int)value->u32);
    // TODO: Store the new network address, then reset after 1 minute (§4.2)
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

/* Key 2 - Network Channel (R/W, Wirepas, 0..39) */
static asProtoStatus_e readNetworkChannel(asProtoValue_t *value)
{
    LOG(LVL_INFO, "READ Network Channel");
    // TODO: Return the current Wirepas network channel
    uint8_t networkChannel;
    lib_settings->getNetworkChannel(&networkChannel);
    value->u32 = networkChannel;
    return AS_PROTO_STATUS_OK;
}

static asProtoStatus_e writeNetworkChannel(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Network Channel: %u", (unsigned int)value->u32);
    // TODO: Store the new network channel, then reset after 1 minute (§4.2)
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

/* Key 3 - Node Role (R/W, Wirepas) */
static asProtoStatus_e validateNodeRole(const asProtoValue_t *value)
{
    // Only the Wirepas stack role values are accepted (app_lib_settings_role_e, §6.1)
    switch (value->u32)
    {
    case APP_LIB_SETTINGS_ROLE_SINK_LE:
    case APP_LIB_SETTINGS_ROLE_SINK_LL:
    case APP_LIB_SETTINGS_ROLE_HEADNODE_LE:
    case APP_LIB_SETTINGS_ROLE_HEADNODE_LL:
    case APP_LIB_SETTINGS_ROLE_SUBNODE_LE:
    case APP_LIB_SETTINGS_ROLE_SUBNODE_LL:
    case APP_LIB_SETTINGS_ROLE_AUTOROLE_LE:
    case APP_LIB_SETTINGS_ROLE_AUTOROLE_LL:
    case APP_LIB_SETTINGS_ROLE_ADVERTISER:
        return AS_PROTO_STATUS_OK;
    default:
        return AS_PROTO_STATUS_OUT_OF_RANGE;
    }
}

static asProtoStatus_e readNodeRole(asProtoValue_t *value)
{
    LOG(LVL_INFO, "READ Node Role");
    // TODO: Return the current Wirepas node role
    uint8_t nodeWirepasRole;
    lib_settings->getNodeRole(&nodeWirepasRole);
    value->u32 = nodeWirepasRole;
    return AS_PROTO_STATUS_OK;
}

static asProtoStatus_e writeNodeRole(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Node Role: 0x%02x", (unsigned int)value->u32);
    // TODO: Store the new node role, then reset after 1 minute (§4.2)
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

/* Key 4 - Network Keys In Use (R/W, Wirepas) */
static asProtoStatus_e readNetworkKeysInUse(asProtoValue_t *value)
{
    LOG(LVL_INFO, "READ Network Keys In Use");
    // TODO: Return whether the network keys are set and active
    value->boolean = lib_settings->getEncryptionKey(NULL) == APP_RES_OK;
    return AS_PROTO_STATUS_OK;
}

static asProtoStatus_e writeNetworkKeysInUse(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Network Keys In Use: %d", value->boolean);
    // TODO: Store the setting, then reset after 1 minute (§4.2)
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

/* Key 5 - Network Cipher Key (W, Wirepas, 16 bytes) */
static asProtoStatus_e writeNetworkCipherKey(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Network Cipher Key (%u bytes)", value->numBytes);
    // TODO: Copy value->bytes (only valid during this call), store it, then reset after 1 minute (§4.2)
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

/* Key 6 - Network Auth Key (W, Wirepas, 16 bytes) */
static asProtoStatus_e writeNetworkAuthKey(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Network Auth Key (%u bytes)", value->numBytes);
    // TODO: Copy value->bytes (only valid during this call), store it, then reset after 1 minute (§4.2)
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

/* Key 7 - Provisioning UID (R/W, Provisioning) */
static asProtoStatus_e readProvisioningUid(asProtoValue_t *value)
{
    LOG(LVL_INFO, "READ Provisioning UID");
    // TODO: Return the provisioning UID
    value->u32 = asProximitySettings.pskId;
    return AS_PROTO_STATUS_OK;
}

static asProtoStatus_e writeProvisioningUid(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Provisioning UID: %u", (unsigned int)value->u32);
    // TODO: Store the provisioning UID
    asProximitySettings.pskId = value->u32;
    asStorageWriteProximitySettings();
    return AS_PROTO_STATUS_OK;
}

/* Key 8 - Provisioning Key (W, Provisioning, 32 bytes) */

static asProtoStatus_e writeProvisioningKey(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Provisioning Key (%u bytes)", value->numBytes);
    // TODO: Copy value->bytes (only valid during this call) and store it
    asProvioningUpdateKeys(value);
    return AS_PROTO_STATUS_OK;
}

/* Key 9 - Heartbeat (R/W, Proximity V1, 60..86400 s) */
static asProtoStatus_e readHeartbeat(asProtoValue_t *value)
{
    LOG(LVL_INFO, "READ Heartbeat");
    // TODO: Return the heartbeat interval in seconds
    value->u32 = asProximitySettings.heartbeatPeriodSeconds;
    return AS_PROTO_STATUS_OK;
}

static asProtoStatus_e writeHeartbeat(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Heartbeat: %u s", (unsigned int)value->u32);
    // TODO: Apply and store the heartbeat interval
    asProximitySettings.heartbeatPeriodSeconds = value->u32;
    asStorageWriteProximitySettings();
    return AS_PROTO_STATUS_OK;
}

/* Key 10 - Anchor Slot (R/W, Proximity V1, 0..22) */
static asProtoStatus_e readAnchorSlot(asProtoValue_t *value)
{
    LOG(LVL_INFO, "READ Anchor Slot");
// TODO: Return the anchor slot
#ifdef AS_RTLS_ROLE_ANCHOR
    value->u32 = asProximitySettings.rstSlotOffset;
    return AS_PROTO_STATUS_OK;
#endif
    // If we get here it means we have a tag and tags do not support this setting
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

static asProtoStatus_e writeAnchorSlot(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Anchor Slot: %u", (unsigned int)value->u32);
// TODO: Apply and store the anchor slot
#ifdef AS_RTLS_ROLE_ANCHOR
    asProximitySettings.rstSlotOffset = value->u32;
    asStorageWriteProximitySettings();
    return AS_PROTO_STATUS_OK;
#endif
    // If we get here it means we have a tag and tags do not support this setting
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

/* Key 11 - Tag Stationary Update Period (R/W, Proximity V1, 1..86400 s) */
static asProtoStatus_e readTagStationaryPeriod(asProtoValue_t *value)
{
    LOG(LVL_INFO, "READ Tag Stationary Update Period");
    // TODO: Return the stationary update period in seconds
#ifdef AS_RTLS_ROLE_TAG
    value->u32 = asProximitySettings.updateRateSeconds;
    return AS_PROTO_STATUS_OK;
#endif
    // If we get here it means we have an Anchor and tags do not support this setting
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

static asProtoStatus_e writeTagStationaryPeriod(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Tag Stationary Update Period: %u s", (unsigned int)value->u32);
// TODO: Apply and store the stationary update period
#ifdef AS_RTLS_ROLE_TAG
    asProximitySettings.updateRateSeconds = value->u32;
    asStorageWriteProximitySettings();
    return AS_PROTO_STATUS_OK;
#endif
    // If we get here it means we have an Anchor and tags do not support this setting
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

/* Key 12 - Tag Motion Update Period (R/W, Proximity V1, 1..86400 s) */
static asProtoStatus_e readTagMotionPeriod(asProtoValue_t *value)
{
    LOG(LVL_INFO, "READ Tag Motion Update Period");
// TODO: Return the motion update period in seconds
#ifdef AS_RTLS_ROLE_TAG
    value->u32 = asProximitySettings.updateRateMotionSeconds;
    return AS_PROTO_STATUS_OK;
#endif
    // If we get here it means we have an Anchor and tags do not support this setting
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

static asProtoStatus_e writeTagMotionPeriod(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Tag Motion Update Period: %u s", (unsigned int)value->u32);
#ifdef AS_RTLS_ROLE_TAG
    asProximitySettings.updateRateMotionSeconds = value->u32;
    asStorageWriteProximitySettings();
    return AS_PROTO_STATUS_OK;
#endif
    // If we get here it means we have an Anchor and tags do not support this setting
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

/* Key 13 - Reset Device Settings / Unprovisioning (W, command, null).
 * The engine already enforces that key 13 is the only data key of the WRITE. */
static asProtoStatus_e writeResetDeviceSettings(const asProtoValue_t *value)
{
    (void)value;
    LOG(LVL_INFO, "COMMAND Reset Device Settings");
    // TODO: After 1 minute, clear Wirepas settings (keys 1-6) only, keep keys 7-8, then restart (§6.2)
    asWirepasResetSettings();
    return AS_PROTO_STATUS_OK;
}

/* Key 14 - Retrieve (W, command, 0 = stop or 10..900 s) */
static asProtoStatus_e validateRetrieve(const asProtoValue_t *value)
{
    // 0 stops the procedure, otherwise 10..900 seconds (§6.2)
    if (value->u32 != 0 && value->u32 < 10)
    {
        return AS_PROTO_STATUS_OUT_OF_RANGE;
    }
    // TODO: Return AS_PROTO_STATUS_UNKNOWN_KEY on devices without LED/buzzer
    return AS_PROTO_STATUS_OK;
}

static asProtoStatus_e writeRetrieve(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "COMMAND Retrieve: %u s", (unsigned int)value->u32);
    // TODO: Start (or restart) the retrieval procedure for value->u32 seconds, or stop it if 0
    if (value->u32 <= MAX_RETRIEVAL_TIME_SECONDS)
    {
#ifdef APP_BUZZER
        asBuzzerRetrieval(value->u32);
#endif
    }
    return AS_PROTO_STATUS_OK;
}

/* Key 15 - Mode (R/W, 0 = Inventory, 1 = Run) */
static asProtoStatus_e readMode(asProtoValue_t *value)
{
    LOG(LVL_INFO, "READ Mode");
    // TODO: Return the current operating mode
    value->u32 = asProximitySettings.mode;
    return AS_PROTO_STATUS_OK;
}

static asProtoStatus_e writeMode(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Mode: %u", (unsigned int)value->u32);
    // TODO: Apply and store the operating mode
    asProximitySettings.mode = (asMode_e)value->u32;
    asStorageWriteProximitySettings();
    return AS_PROTO_STATUS_OK;
}

/* Key 16 - Ping (W, command, null) */
static asProtoStatus_e writePing(const asProtoValue_t *value)
{
    (void)value;
    LOG(LVL_INFO, "COMMAND Ping");
    // Nothing to do: the OK response is the acknowledgement (§6.2)
    return AS_PROTO_STATUS_OK;
}

/* Key 17 - UWB Operation Enabled (R/W, Device, Boolean) */
static asProtoStatus_e readUwbOperationEnabled(asProtoValue_t *value)
{
    LOG(LVL_INFO, "READ UWB Operation Enabled");
    // TODO: Return whether UWB operation is enabled
#ifdef APP_UWB
    value->boolean = asProximitySettings.uwbOperationEnabled;
    return AS_PROTO_STATUS_OK;
#endif
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

static asProtoStatus_e writeUwbOperationEnabled(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE UWB Operation Enabled: %d", value->boolean);
// TODO: Apply and store the UWB operation setting
#ifdef APP_UWB
    asProximitySettings.uwbOperationEnabled = value->boolean;
    asStorageWriteProximitySettings();
    return AS_PROTO_STATUS_OK;
#endif
    return AS_PROTO_STATUS_UNSUPPORTED_OP;
}

/* Key 18 - Device Management Enabled (R/W, Device, Boolean) */
static asProtoStatus_e readDeviceManagementEnabled(asProtoValue_t *value)
{
    LOG(LVL_INFO, "READ Device Management Enabled");
    // TODO: Return whether device management is enabled
    value->boolean = asProximitySettings.deviceManagementEnabled;
    return AS_PROTO_STATUS_OK;
}

static asProtoStatus_e writeDeviceManagementEnabled(const asProtoValue_t *value)
{
    LOG(LVL_INFO, "WRITE Device Management Enabled: %d", value->boolean);
    // TODO: Apply and store the device management setting
    asProximitySettings.deviceManagementEnabled = value->boolean;
    asStorageWriteProximitySettings();
    return AS_PROTO_STATUS_OK;
}

/******************************************************************************
 * Key Database (§6). A NULL read callback makes a key write-only.
 *****************************************************************************/
static const asProtoKeyInfo_t asProtoKeyTable[] = {
    /* key,                              type,                min, max,        read,                    write,                    validate,         name */
    {AS_PROTO_KEY_NETWORK_ADDRESS, AS_PROTO_TYPE_UINT, 0, 0xFFFFFF, readNetworkAddress, writeNetworkAddress, NULL, "Network Address"},
    {AS_PROTO_KEY_NETWORK_CHANNEL, AS_PROTO_TYPE_UINT, 0, 39, readNetworkChannel, writeNetworkChannel, NULL, "Network Channel"},
    {AS_PROTO_KEY_NODE_ROLE, AS_PROTO_TYPE_UINT, 0, 0xFF, readNodeRole, writeNodeRole, validateNodeRole, "Node Role"},
    {AS_PROTO_KEY_NETWORK_KEYS_IN_USE, AS_PROTO_TYPE_BOOL, 0, 0, readNetworkKeysInUse, writeNetworkKeysInUse, NULL, "Network Keys In Use"},
    {AS_PROTO_KEY_NETWORK_CIPHER_KEY, AS_PROTO_TYPE_BYTES, 0, 16, NULL, writeNetworkCipherKey, NULL, "Network Cipher Key"},
    {AS_PROTO_KEY_NETWORK_AUTH_KEY, AS_PROTO_TYPE_BYTES, 0, 16, NULL, writeNetworkAuthKey, NULL, "Network Auth Key"},
    {AS_PROTO_KEY_PROVISIONING_UID, AS_PROTO_TYPE_UINT, 0, 0xFFFFFFFF, readProvisioningUid, writeProvisioningUid, NULL, "Provisioning UID"},
    {AS_PROTO_KEY_PROVISIONING_KEY, AS_PROTO_TYPE_BYTES, 0, 32, NULL, writeProvisioningKey, NULL, "Provisioning Key"},
    {AS_PROTO_KEY_HEARTBEAT, AS_PROTO_TYPE_UINT, 60, 86400, readHeartbeat, writeHeartbeat, NULL, "Heartbeat"},
    {AS_PROTO_KEY_ANCHOR_SLOT, AS_PROTO_TYPE_UINT, 0, 22, readAnchorSlot, writeAnchorSlot, NULL, "Anchor Slot"},
    {AS_PROTO_KEY_TAG_STATIONARY_PERIOD, AS_PROTO_TYPE_UINT, 1, 86400, readTagStationaryPeriod, writeTagStationaryPeriod, NULL, "Tag Stationary Update Period"},
    {AS_PROTO_KEY_TAG_MOTION_PERIOD, AS_PROTO_TYPE_UINT, 1, 86400, readTagMotionPeriod, writeTagMotionPeriod, NULL, "Tag Motion Update Period"},
    {AS_PROTO_KEY_RESET_DEVICE_SETTINGS, AS_PROTO_TYPE_NULL, 0, 0, NULL, writeResetDeviceSettings, NULL, "Reset Device Settings"},
    {AS_PROTO_KEY_RETRIEVE, AS_PROTO_TYPE_UINT, 0, 900, NULL, writeRetrieve, validateRetrieve, "Retrieve"},
    {AS_PROTO_KEY_MODE, AS_PROTO_TYPE_UINT, 0, 1, readMode, writeMode, NULL, "Mode"},
    {AS_PROTO_KEY_PING, AS_PROTO_TYPE_NULL, 0, 0, NULL, writePing, NULL, "Ping"},
    {AS_PROTO_KEY_UWB_OPERATION_ENABLED, AS_PROTO_TYPE_BOOL, 0, 0, readUwbOperationEnabled, writeUwbOperationEnabled, NULL, "UWB Operation Enabled"},
    {AS_PROTO_KEY_DEVICE_MANAGEMENT_ENABLED, AS_PROTO_TYPE_BOOL, 0, 0, readDeviceManagementEnabled, writeDeviceManagementEnabled, NULL, "Device Management Enabled"},
};

static const uint8_t asProtoKeyTableSize = sizeof(asProtoKeyTable) / sizeof(asProtoKeyTable[0]);

/******************************************************************************
 * Protocol engine
 *****************************************************************************/

/* Upper bound on the data keys of one WRITE (duplicates are rejected, so the key table size is enough) */
#define AS_PROTO_MAX_WRITE_KEYS (sizeof(asProtoKeyTable) / sizeof(asProtoKeyTable[0]))

/* Control fields and summary of a decoded request */
typedef struct
{
    bool hasOperation;
    bool hasSequence;
    bool hasVersion;
    bool hasKeyList;
    uint64_t operation;
    uint16_t sequence;
    uint64_t version;
    size_t keyListPos;   /* Reader offset of the -4 array */
    uint8_t numDataKeys; /* Number of positive keys in the map */
} asProtoRequest_t;

/* A validated WRITE entry waiting to be applied */
typedef struct
{
    const asProtoKeyInfo_t *info;
    asProtoValue_t value;
} asProtoPendingWrite_t;

/* Response parts, built separately because the map headers need the entry counts up front */
typedef struct
{
    cborWriter_t data;   /* Data key/value pairs (READ) */
    cborWriter_t errors; /* Key Errors {key -> status} pairs */
    uint8_t numData;
    uint8_t numErrors;
    asProtoStatus_e firstError;
} asProtoResponse_t;

static uint8_t responseDataBuf[AS_PROTOCOL_MAX_MSG_SIZE];
static uint8_t responseErrorsBuf[AS_PROTOCOL_MAX_MSG_SIZE];
static asProtoPendingWrite_t pendingWrites[AS_PROTO_MAX_WRITE_KEYS];

static const asProtoKeyInfo_t *asProtoFindKey(uint64_t key)
{
    for (uint8_t i = 0; i < asProtoKeyTableSize; i++)
    {
        if (asProtoKeyTable[i].key == key)
        {
            return &asProtoKeyTable[i];
        }
    }
    return NULL;
}

static void asProtoResponseInit(asProtoResponse_t *rsp)
{
    cborWriterInit(&rsp->data, responseDataBuf, sizeof(responseDataBuf));
    cborWriterInit(&rsp->errors, responseErrorsBuf, sizeof(responseErrorsBuf));
    rsp->numData = 0;
    rsp->numErrors = 0;
    rsp->firstError = AS_PROTO_STATUS_OK;
}

static void asProtoAddKeyError(asProtoResponse_t *rsp, uint64_t key, asProtoStatus_e status)
{
    LOG(LVL_WARNING, "Key %u rejected, status %d", (unsigned int)key, status);

    if (rsp->numErrors == 0)
    {
        rsp->firstError = status;
    }
    cborPutUint(&rsp->errors, key);
    cborPutUint(&rsp->errors, status);
    rsp->numErrors++;
}

/**
 * \brief   Encodes the final RESPONSE map: {-1: 3, -2: seq, -3: status, <data>, -5: {<errors>}, -6: version}
 * \return  Encoded length, 0 if it doesn't fit
 */
static size_t asProtoEncodeResponse(uint8_t *out, size_t outSize, uint16_t sequence, asProtoStatus_e status,
                                    const asProtoResponse_t *rsp, bool includeVersion)
{
    cborWriter_t w;
    uint8_t numData = rsp ? rsp->numData : 0;
    uint8_t numErrors = rsp ? rsp->numErrors : 0;

    cborWriterInit(&w, out, outSize);
    cborPutMapHeader(&w, 3 + numData + (numErrors ? 1 : 0) + (includeVersion ? 1 : 0));
    cborPutInt(&w, AS_PROTO_CTRL_OPERATION);
    cborPutUint(&w, AS_PROTO_OP_RESPONSE);
    cborPutInt(&w, AS_PROTO_CTRL_SEQUENCE);
    cborPutUint(&w, sequence);
    cborPutInt(&w, AS_PROTO_CTRL_STATUS);
    cborPutUint(&w, status);
    if (numData)
    {
        cborPutRaw(&w, rsp->data.buf, rsp->data.pos);
    }
    if (numErrors)
    {
        cborPutInt(&w, AS_PROTO_CTRL_KEY_ERRORS);
        cborPutMapHeader(&w, numErrors);
        cborPutRaw(&w, rsp->errors.buf, rsp->errors.pos);
    }
    if (includeVersion)
    {
        cborPutInt(&w, AS_PROTO_CTRL_VERSION);
        cborPutUint(&w, AS_PROTOCOL_VERSION);
    }

    if (w.overflow || w.pos > AS_PROTOCOL_MAX_MSG_SIZE)
    {
        return 0;
    }
    return w.pos;
}

/**
 * \brief   Encodes a RESPONSE carrying only the control fields
 */
static size_t asProtoEncodeStatus(uint8_t *out, size_t outSize, uint16_t sequence, asProtoStatus_e status)
{
    LOG(LVL_INFO, "Response seq %u status %d", sequence, status);
    return asProtoEncodeResponse(out, outSize, sequence, status, NULL, status == AS_PROTO_STATUS_UNSUPPORTED_VERSION);
}

/**
 * \brief   Encodes a full RESPONSE, falling back to MSG_TOO_LARGE if it doesn't fit in 100 bytes
 */
static size_t asProtoEncodeFull(uint8_t *out, size_t outSize, uint16_t sequence, asProtoStatus_e status,
                                const asProtoResponse_t *rsp)
{
    size_t len = 0;

    if (!rsp->data.overflow && !rsp->errors.overflow)
    {
        len = asProtoEncodeResponse(out, outSize, sequence, status, rsp, false);
    }
    if (len == 0)
    {
        LOG(LVL_WARNING, "Response does not fit in %d bytes", AS_PROTOCOL_MAX_MSG_SIZE);
        return asProtoEncodeStatus(out, outSize, sequence, AS_PROTO_STATUS_MSG_TOO_LARGE);
    }

    LOG(LVL_INFO, "Response seq %u status %d, %u values, %u key errors", sequence, status, rsp->numData, rsp->numErrors);
    return len;
}

/**
 * \brief   First pass over the request map: extracts the control fields and checks the map is well formed
 */
static asProtoStatus_e asProtoParseControl(const uint8_t *request, size_t requestLen, asProtoRequest_t *req)
{
    cborReader_t r;
    uint8_t major, ai;
    uint64_t numPairs;
    uint64_t seenDataKeys = 0;    /* Duplicate detection for keys 0..63 */
    uint32_t seenControlKeys = 0; /* Duplicate detection for keys -1..-24 */

    memset(req, 0, sizeof(*req));
    cborReaderInit(&r, request, requestLen);

    if (!cborReadHead(&r, &major, &ai, &numPairs) || major != CBOR_MAJOR_MAP)
    {
        return AS_PROTO_STATUS_MALFORMED;
    }

    for (uint64_t i = 0; i < numPairs; i++)
    {
        int64_t key;
        cborValue_t value;

        if (!cborReadInt(&r, &key))
        {
            return AS_PROTO_STATUS_MALFORMED;
        }

        if (key >= 0)
        {
            if (key < 64)
            {
                if (seenDataKeys & (1ULL << key))
                {
                    return AS_PROTO_STATUS_MALFORMED;
                }
                seenDataKeys |= (1ULL << key);
            }
            req->numDataKeys++;
            if (!cborSkip(&r))
            {
                return AS_PROTO_STATUS_MALFORMED;
            }
            continue;
        }

        if (key >= -24)
        {
            if (seenControlKeys & (1UL << (-key - 1)))
            {
                return AS_PROTO_STATUS_MALFORMED;
            }
            seenControlKeys |= (1UL << (-key - 1));
        }

        if (key == AS_PROTO_CTRL_KEY_LIST)
        {
            req->keyListPos = r.pos;
        }

        if (!cborReadValue(&r, &value))
        {
            return AS_PROTO_STATUS_MALFORMED;
        }

        switch (key)
        {
        case AS_PROTO_CTRL_OPERATION:
            if (value.type != CBOR_TYPE_UINT)
            {
                return AS_PROTO_STATUS_MALFORMED;
            }
            req->operation = value.arg;
            req->hasOperation = true;
            break;
        case AS_PROTO_CTRL_SEQUENCE:
            if (value.type != CBOR_TYPE_UINT || value.arg > 0xFFFF)
            {
                return AS_PROTO_STATUS_MALFORMED;
            }
            req->sequence = (uint16_t)value.arg;
            req->hasSequence = true;
            break;
        case AS_PROTO_CTRL_VERSION:
            if (value.type != CBOR_TYPE_UINT)
            {
                return AS_PROTO_STATUS_MALFORMED;
            }
            req->version = value.arg;
            req->hasVersion = true;
            break;
        case AS_PROTO_CTRL_KEY_LIST:
            if (value.type != CBOR_TYPE_ARRAY)
            {
                return AS_PROTO_STATUS_MALFORMED;
            }
            req->hasKeyList = true;
            break;
        default:
            // Unknown or unused control keys are ignored (§3 rule 6)
            break;
        }
    }

    // Exactly one CBOR item per payload (§9.2)
    if (!cborReaderAtEnd(&r))
    {
        return AS_PROTO_STATUS_MALFORMED;
    }

    if (!req->hasOperation || !req->hasSequence)
    {
        return AS_PROTO_STATUS_MALFORMED;
    }

    return AS_PROTO_STATUS_OK;
}

static void asProtoEncodeValue(cborWriter_t *w, const asProtoKeyInfo_t *info, const asProtoValue_t *value)
{
    switch (info->type)
    {
    case AS_PROTO_TYPE_UINT:
        cborPutUint(w, value->u32);
        break;
    case AS_PROTO_TYPE_BOOL:
        cborPutBool(w, value->boolean);
        break;
    case AS_PROTO_TYPE_BYTES:
        cborPutBytes(w, value->bytes, value->numBytes);
        break;
    case AS_PROTO_TYPE_NULL:
    default:
        cborPutNull(w);
        break;
    }
}

/**
 * \brief   Handles a READ: returns every requested key that can be read, lists the others in -5 (§4.2)
 */
static size_t asProtoHandleRead(const uint8_t *request, size_t requestLen, const asProtoRequest_t *req,
                                uint8_t *response, size_t responseSize)
{
    asProtoResponse_t rsp;
    cborReader_t r;
    uint8_t major, ai;
    uint64_t numKeys;
    asProtoStatus_e status;

    if (!req->hasKeyList)
    {
        return asProtoEncodeStatus(response, responseSize, req->sequence, AS_PROTO_STATUS_MALFORMED);
    }

    asProtoResponseInit(&rsp);
    cborReaderInit(&r, request, requestLen);
    r.pos = req->keyListPos;
    cborReadHead(&r, &major, &ai, &numKeys); // Already validated as an array by asProtoParseControl

    for (uint64_t i = 0; i < numKeys; i++)
    {
        cborValue_t keyItem;
        const asProtoKeyInfo_t *info;
        asProtoValue_t value;

        if (!cborReadValue(&r, &keyItem) || keyItem.type != CBOR_TYPE_UINT)
        {
            return asProtoEncodeStatus(response, responseSize, req->sequence, AS_PROTO_STATUS_MALFORMED);
        }

        info = asProtoFindKey(keyItem.arg);
        if (info == NULL)
        {
            asProtoAddKeyError(&rsp, keyItem.arg, AS_PROTO_STATUS_UNKNOWN_KEY);
            continue;
        }
        if (info->read == NULL)
        {
            asProtoAddKeyError(&rsp, keyItem.arg, AS_PROTO_STATUS_ACCESS_DENIED);
            continue;
        }

        memset(&value, 0, sizeof(value));
        status = info->read(&value);
        if (status != AS_PROTO_STATUS_OK)
        {
            asProtoAddKeyError(&rsp, keyItem.arg, status);
            continue;
        }

        cborPutUint(&rsp.data, info->key);
        asProtoEncodeValue(&rsp.data, info, &value);
        rsp.numData++;
    }

    if (rsp.numErrors == 0)
    {
        status = AS_PROTO_STATUS_OK;
    }
    else if (rsp.numData > 0)
    {
        status = AS_PROTO_STATUS_PARTIAL;
    }
    else
    {
        status = rsp.firstError;
    }

    return asProtoEncodeFull(response, responseSize, req->sequence, status, &rsp);
}

/**
 * \brief   Checks a WRITE value against the key's type and range. Changes nothing on the device.
 */
static asProtoStatus_e asProtoValidateWrite(const asProtoKeyInfo_t *info, const cborValue_t *item, asProtoValue_t *value)
{
    memset(value, 0, sizeof(*value));

    if (info->write == NULL)
    {
        return AS_PROTO_STATUS_ACCESS_DENIED;
    }

    switch (info->type)
    {
    case AS_PROTO_TYPE_UINT:
        if (item->type != CBOR_TYPE_UINT)
        {
            return AS_PROTO_STATUS_INVALID_TYPE;
        }
        if (item->arg < info->minValue || item->arg > info->maxValue)
        {
            return AS_PROTO_STATUS_OUT_OF_RANGE;
        }
        value->u32 = (uint32_t)item->arg;
        break;
    case AS_PROTO_TYPE_BOOL:
        if (item->type != CBOR_TYPE_BOOL)
        {
            return AS_PROTO_STATUS_INVALID_TYPE;
        }
        value->boolean = item->boolean;
        break;
    case AS_PROTO_TYPE_BYTES:
        if (item->type != CBOR_TYPE_BYTES || item->arg != info->maxValue)
        {
            return AS_PROTO_STATUS_INVALID_TYPE;
        }
        value->bytes = item->bytes;
        value->numBytes = (uint8_t)item->arg;
        break;
    case AS_PROTO_TYPE_NULL:
    default:
        if (item->type != CBOR_TYPE_NULL)
        {
            return AS_PROTO_STATUS_INVALID_TYPE;
        }
        break;
    }

    if (info->validate != NULL)
    {
        return info->validate(value);
    }
    return AS_PROTO_STATUS_OK;
}

/**
 * \brief   Handles a WRITE. All keys are validated first; nothing is applied unless all of them pass (§4.2).
 */
static size_t asProtoHandleWrite(const uint8_t *request, size_t requestLen, const asProtoRequest_t *req,
                                 uint8_t *response, size_t responseSize)
{
    asProtoResponse_t rsp;
    cborReader_t r;
    uint8_t major, ai;
    uint64_t numPairs;
    uint8_t numPending = 0;
    asProtoStatus_e status;

    asProtoResponseInit(&rsp);
    cborReaderInit(&r, request, requestLen);
    cborReadHead(&r, &major, &ai, &numPairs); // Already validated by asProtoParseControl

    // Pass 1: validate every data key
    for (uint64_t i = 0; i < numPairs; i++)
    {
        int64_t key;
        cborValue_t item;
        const asProtoKeyInfo_t *info;
        asProtoValue_t value;

        cborReadInt(&r, &key);
        cborReadValue(&r, &item);

        if (key < 0)
        {
            continue; // Control field
        }

        info = asProtoFindKey((uint64_t)key);
        if (info == NULL)
        {
            asProtoAddKeyError(&rsp, (uint64_t)key, AS_PROTO_STATUS_UNKNOWN_KEY);
            continue;
        }

        status = asProtoValidateWrite(info, &item, &value);

        // Key 13 must be the only data key in its WRITE (§6.2)
        if (status == AS_PROTO_STATUS_OK && key == AS_PROTO_KEY_RESET_DEVICE_SETTINGS && req->numDataKeys > 1)
        {
            status = AS_PROTO_STATUS_INVALID_TYPE;
        }

        if (status == AS_PROTO_STATUS_OK && numPending >= AS_PROTO_MAX_WRITE_KEYS)
        {
            status = AS_PROTO_STATUS_BUSY;
        }

        if (status != AS_PROTO_STATUS_OK)
        {
            asProtoAddKeyError(&rsp, (uint64_t)key, status);
            continue;
        }

        pendingWrites[numPending].info = info;
        pendingWrites[numPending].value = value;
        numPending++;
    }

    if (rsp.numErrors > 0)
    {
        LOG(LVL_WARNING, "WRITE rejected, nothing applied");
        return asProtoEncodeFull(response, responseSize, req->sequence, rsp.firstError, &rsp);
    }

    // Pass 2: everything is valid, apply
    for (uint8_t i = 0; i < numPending; i++)
    {
        const asProtoKeyInfo_t *info = pendingWrites[i].info;

        status = info->write(&pendingWrites[i].value);
        if (status != AS_PROTO_STATUS_OK)
        {
            asProtoAddKeyError(&rsp, info->key, status);
        }
    }

    return asProtoEncodeFull(response, responseSize, req->sequence, rsp.firstError, &rsp);
}

size_t asProtocolProcessRequest(const uint8_t *request, size_t requestLen, uint8_t *response, size_t responseSize)
{
    asProtoRequest_t req;
    asProtoStatus_e status;

    if (responseSize < AS_PROTOCOL_MAX_MSG_SIZE)
    {
        return 0;
    }

    if (requestLen > AS_PROTOCOL_MAX_MSG_SIZE)
    {
        LOG(LVL_WARNING, "Request too large: %u bytes", (unsigned int)requestLen);
        return asProtoEncodeStatus(response, responseSize, 0, AS_PROTO_STATUS_MSG_TOO_LARGE);
    }

    status = asProtoParseControl(request, requestLen, &req);
    if (status != AS_PROTO_STATUS_OK)
    {
        LOG(LVL_WARNING, "Malformed request");
        // Echo the sequence if it was recovered, otherwise 0 (§9.2)
        return asProtoEncodeStatus(response, responseSize, req.hasSequence ? req.sequence : 0, status);
    }

    LOG(LVL_INFO, "Request op %u seq %u, %u data keys", (unsigned int)req.operation, req.sequence, req.numDataKeys);

    if (req.hasVersion && req.version != AS_PROTOCOL_VERSION)
    {
        return asProtoEncodeStatus(response, responseSize, req.sequence, AS_PROTO_STATUS_UNSUPPORTED_VERSION);
    }

    switch (req.operation)
    {
    case AS_PROTO_OP_READ:
        return asProtoHandleRead(request, requestLen, &req, response, responseSize);
    case AS_PROTO_OP_WRITE:
        return asProtoHandleWrite(request, requestLen, &req, response, responseSize);
    default:
        return asProtoEncodeStatus(response, responseSize, req.sequence, AS_PROTO_STATUS_UNSUPPORTED_OP);
    }
}
