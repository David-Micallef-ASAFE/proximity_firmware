/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_uwb.c
 * \brief   This file handles all the UWB related logic
 */

#include <stdlib.h>

#include "api.h"
#include "node_configuration.h"
#include "shared_data.h"
#include "app_scheduler.h"

#include "board.h"

#include "as_spi.h"

#include "gpio.h"

#include "port_platform.h"
#include "deca_device_api.h"
// #include "deca_regs.h"
#include "dw3000_deca_regs.h"
#include "dw3000_deca_vals.h"
#include "deca_probe_interface.h"

#include "as_uwb.h"
#include "as_ranging.h"
#include "as_nbor.h"
#include "as_uwbTimer.h"

#include "math.h"

#define DW3000_A_PRF16 113.8f
#define DW3000_A_PRF64 121.7f
#define DW3000_RSSI_INVALID INT16_MIN

#define DEBUG_LOG_MODULE_NAME "AS UWB"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
    5,                /* Channel number. */
    DWT_PLEN_128,     /* Preamble length. Used in TX only. */
    DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
    9,                /* TX preamble code. Used in TX only. */
    9,                /* RX preamble code. Used in RX only. */
    0,                /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,       /* Data rate. */
    DWT_PHRMODE_STD,  /* PHY header mode. */
    DWT_PHRRATE_STD,  /* PHY header rate. */
    (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF, /* No STS mode enabled (STS Mode 0). */
    DWT_STS_LEN_64,   /* STS length, see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0       /* PDOA mode off */
};

#define UWB_PAN_ID 0x6E67

/*Discovery Mesage Definitions*/

static uint8_t uwbTXMessageBuffer[] = {
    0x41, 0xCC,             // Frame Control
    0,                      // Sequence Number
    0x67, 0x6E,             // PAN ID
    0x00, 0x01, 0x02, 0x03, // Destination Address
    0xAA, 0xBB, 0xCC, 0xDD, // Source Address
    0x01,                   // Function code
    0x51, 0x52, 0x53, 0x54, // Ticks to Ranging Slot
    0x00,                   // Nbor Anchors
    0x00, 0x00              // Checksum
};

/* Buffer to store received response message*/
static uint8 uwbRXMessageBuffer[sizeof(uwbTXMessageBuffer)];

#define UWB_MSG_IDX_SEQUENCE 2
#define UWB_MSG_IDX_PAN_ID 3
#define UWB_MSG_IDX_DST_ADDRESS 5
#define UWB_MSG_IDX_SRC_ADDRESS 9
#define UWB_MSG_IDX_FUNCTION_CODE 13
#define UWB_MSG_IDX_RS_TICKS 14
#define UWB_MSG_IDX_DETECTED_NBOR_ANCHORS 18

uint32_t deviceAddress;
uwbFunction_e currentFunction;
extern bool enableRxAfterTx; // Defined in Timer

/* Buffer to store received response message*/
// static uint8_t uwbRXMessageBuffer[sizeof(uwbTXMessageBuffer)];

static uint32_t asUWBProcessCallbacks()
{
    dwt_isr();
    return APP_SCHEDULER_STOP_TASK;
}

static void uwb_event_cb(gpio_id_t id, gpio_in_event_e event)
{
    (void)id;
    (void)event;
    // LOG(LVL_INFO, "UWB Interrupt");
    // dwt_isr();
    App_Scheduler_addTask_execTime(asUWBProcessCallbacks, APP_SCHEDULER_SCHEDULE_ASAP, 500);
}

static void enableUWBInterrupt()
{

    gpio_res_e gpio_res;
    const gpio_in_cfg_t uwb_int_cfg =
        {
            .event_cb = uwb_event_cb,
            .event_cfg = GPIO_IN_EVENT_RISING_EDGE,
            .in_mode_cfg = GPIO_IN_PULL_NONE};

    gpio_res = Gpio_inputSetCfg(BOARD_GPIO_ID_DW_IRQ, &uwb_int_cfg);

    if (gpio_res != GPIO_RES_OK)
    {

        LOG(LVL_INFO, "Cannot enable interrupt pin %d", gpio_res);
    }
    else
    {
        LOG(LVL_INFO, "UWB INT pin  registered");
    }
}

// static void disableUWBInterrupt()
// {
//     gpio_res_e gpio_res;
//     const gpio_in_cfg_t uwb_int_cfg =
//         {
//             .event_cb = NULL,
//             .event_cfg = GPIO_IN_EVENT_NONE,
//             .in_mode_cfg = GPIO_IN_PULL_NONE};

//     gpio_res = Gpio_inputSetCfg(BOARD_GPIO_ID_DW_IRQ, &uwb_int_cfg);

//     if (gpio_res != GPIO_RES_OK)
//     {

//         LOG(LVL_INFO, "Cannot disable interrupt pin %d", gpio_res);
//     }
//     else
//     {
//         LOG(LVL_INFO, "UWB INT pin  deregistered");
//     }
// }

// int16_t rssiRaw;
// dwt_cirdiags_t cirDiags;

// static void getSignalMetrics3k(uint8_t *rssi)
// {

//     int8_t rssiByte;

//     // RSSI
//     dwt_readdiagnostics_acc(&cirDiags, DWT_ACC_IDX_IP_M);
//     dwt_calculate_rssi(&cirDiags, DWT_ACC_IDX_IP_M, &rssiRaw);
//     rssiByte = rssiRaw >> 8;
//     rssiByte += 120;
//     LOG(LVL_INFO, "RSSI %d %d, %d", rssiRaw, rssiByte, cirDiags.accumCount);
//     *rssi = (uint8_t)rssiByte;
// }

static int dw3000_read_rssi(uint8_t prf, int16_t *rssi_dbm, int16_t *fp_dbm)
{
    dwt_rxdiag_t diag;
    dwt_readdiagnostics(&diag);

    const float C = (float)diag.ipatovPower;
    const float N = (float)diag.ipatovAccumCount;
    const uint8_t D = dwt_get_dgcdecision();
    const float A = (prf == DWT_PRF_16M) ? DW3000_A_PRF16 : DW3000_A_PRF64;

    if (C <= 0.0f || N <= 0.0f)
    {
        *rssi_dbm = DW3000_RSSI_INVALID;
        if (fp_dbm)
            *fp_dbm = DW3000_RSSI_INVALID;
        return -1;
    }

    const float two21 = 2097152.0f; /* 2^21 */
    const float n2 = N * N;

    float rssi = 10.0f * log10f((C * two21) / n2) + 6.0f * D - A;
    *rssi_dbm = (int16_t)lroundf(rssi);

    if (fp_dbm)
    {
        const float f1 = (float)diag.ipatovF1;
        const float f2 = (float)diag.ipatovF2;
        const float f3 = (float)diag.ipatovF3;
        const float fp_sum = f1 * f1 + f2 * f2 + f3 * f3;

        if (fp_sum > 0.0f)
        {
            float fp = 10.0f * log10f((fp_sum * two21) / n2) + 6.0f * D - A;
            *fp_dbm = (int16_t)lroundf(fp);
        }
        else
        {
            *fp_dbm = DW3000_RSSI_INVALID;
        }
    }

    LOG(LVL_INFO, "CL RSSI %d %d", *rssi_dbm, *fp_dbm);
    return 0;
}

/*Callbacks*/
/*Callback to process TX confirmation events*/
void tx_conf_cb(const dwt_cb_data_t *cb_data)
{
    LOG(LVL_INFO, "TX Ok");
#ifdef APP_LOGIC_DEBUG
    nrf_gpio_pin_clear(SIGNAL_A_PIN);
    nrf_gpio_pin_clear(SIGNAL_B_PIN);
#endif

    // Store the tx timestamp for RSS and RSE
    switch (currentFunction)
    {
    case UWB_FUNCTION_CODE_RSS:
        rangingSlotRSS(0, dwt_readtxtimestamplo32(), 0, 0, 0);
        break;
    case UWB_FUNCTION_CODE_RSE:
        rangingSlotRSE(dwt_readtxtimestamplo32());
        break;
    default:
        break;
    }

    /*DW3000 control*/
    if (enableRxAfterTx)
    {
        enableRxAfterTx = false;
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
    }
}

/*Callback to process RX error events*/
void rx_err_cb(const dwt_cb_data_t *cb_data)
{
    LOG(LVL_INFO, "RX Error");
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

/*Callback to process RX timeout events*/
void rx_to_cb(const dwt_cb_data_t *cb_data)
{
    LOG(LVL_INFO, "RX Timeout Error");
}
/*Callback to process RX good frame events*/
void rx_ok_cb(const dwt_cb_data_t *cb_data)
{
    /* Perform manual RX re-enabling. See NOTE 5 below. */
    uint16_t *panID;
    // uint32_t *dstAddress;
    uint32_t *srcAddress;
    // dwt_rxdiag_t diagnostics;
    uint32_t *nextRSUs;
    uint8_t rssi; //, confidenceScore;
    int16_t rssi_c, fp_c;
    uint32_t *uwbTimestamp; //, *uwbTimestampHigh;
    uint8_t timestampBuffer[5];

    dwt_rxenable(DWT_START_RX_IMMEDIATE);
    LOG(LVL_INFO, "RX OK");

    /* A frame has been received, copy it to our local buffer. */
    if (cb_data->datalength <= sizeof(uwbRXMessageBuffer))
    {
        // Read the data
        dwt_readrxtimestamp(timestampBuffer, DWT_IP_M);
        uwbTimestamp = (uint32_t *)&timestampBuffer[0];
        // uwbTimestampHigh = (uint32_t *)&timestampBuffer[1];

        dwt_readrxdata(uwbRXMessageBuffer, cb_data->datalength, 0);

        panID = (uint16_t *)&uwbRXMessageBuffer[UWB_MSG_IDX_PAN_ID];
        // dstAddress = (uint32_t *)&uwbRXMessageBuffer[UWB_MSG_IDX_DST_ADDRESS];
        srcAddress = (uint32_t *)&uwbRXMessageBuffer[UWB_MSG_IDX_SRC_ADDRESS];

        // Check if the message is within our PAN ID
        if (*panID == UWB_PAN_ID)
        {
            LOG(LVL_INFO, "RX OK %d %d", *srcAddress, uwbRXMessageBuffer[UWB_MSG_IDX_DETECTED_NBOR_ANCHORS]);

            dw3000_read_rssi(DWT_PRF_64M, &rssi_c, &fp_c);
            rssi = rssi_c + 120;

            // Get the next RS info
            // We need to calculate, the timer tick that started this current operation
            nextRSUs = (uint32_t *)&uwbRXMessageBuffer[UWB_MSG_IDX_RS_TICKS]; // This will now indicate slots

            nborManagerAddNborThroughUWB(*srcAddress, *nextRSUs, 0, uwbRXMessageBuffer[UWB_MSG_IDX_DETECTED_NBOR_ANCHORS], rssi);
            switch (uwbRXMessageBuffer[UWB_MSG_IDX_FUNCTION_CODE])
            {
            case UWB_FUNCTION_CODE_RST: // Expected to be received during a ranging
                rangingAddAnchor(*srcAddress, *uwbTimestamp, rssi, 0);
                // LOG(LVL_INFO, "RST");
                break;
            case UWB_FUNCTION_CODE_RSS:
#ifdef AS_RTLS_ROLE_TAG
                rangingSlotRSS(*srcAddress, *uwbTimestamp, uwbRXMessageBuffer[UWB_MSG_IDX_SEQUENCE], rssi, 0);
                LOG(LVL_INFO, "RSS %u", *uwbTimestamp);
#endif
#ifdef AS_RTLS_ROLE_ANCHOR
                // Collision
                asUWBTimerInidicateCollisionDetected();
#endif
                break;
            case UWB_FUNCTION_CODE_RSE:
#ifdef AS_RTLS_ROLE_TAG
                rangingSlotRSE(*uwbTimestamp);
                LOG(LVL_INFO, "RSE %u", *nextRSUs);
                uwbDisableRX(); // Since we do not need any further
#endif
#ifdef AS_RTLS_ROLE_ANCHOR
                // Collision
                asUWBTimerInidicateCollisionDetected();
#endif
                break;
            case UWB_FUNCTION_CODE_NBOR_DISCOVERY:
                // Nothing required as we are already adding through UWB
                break;
            }
        }
    }
}

void asUWBSleep()
{
    dwt_entersleep(DWT_DW_INIT);
}

void asUWBEnableInterrupt()
{
    enableUWBInterrupt();
}

void asUWBInit()
{
    uint32_t *addrPtr;
    // int32_t res;
    dwt_callbacks_s cbs = {NULL};

    /*Initialize the GPIOs used in SPI*/
    asSPIInit();

    // RSTn must never be driven high externally (the DW3000 drives it itself). Leave it released;
    // reset_DW1000() only drives it low for the reset pulse.
    nrf_gpio_cfg_input(DW_RST_PIN, NRF_GPIO_PIN_NOPULL);

    /*Wakeup the DW1000*/
    port_wakeup_dw1000();

    /* Reset DW1000 */
    reset_DW1000();

    port_set_dw1000_slowrate();

    dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

    /* Init the DW1000 */
    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    {
        LOG(LVL_INFO, "DW3000 Error");
        return;
    }

    LOG(LVL_INFO, "DW3000 OK");

    // Set SPI to 8MHz clock
    port_set_dw1000_fastrate();

    /* Configure DW1000. */
    dwt_configure(&config);

    /* Activate double buffering. */
    dwt_setdblrxbuffmode(1, DBL_BUF_MODE_MAN);

    /*Setup Callbacks*/
    cbs.cbTxDone = tx_conf_cb;
    cbs.cbRxOk = rx_ok_cb;
    cbs.cbRxTo = rx_to_cb;
    cbs.cbRxErr = rx_err_cb;
    dwt_setcallbacks(&cbs);

    /*Interrupts*/
    enableUWBInterrupt();
    dwt_setinterrupt(DWT_INT_TXFRS_BIT_MASK | DWT_INT_RXFCG_BIT_MASK | DWT_INT_RXFTO_BIT_MASK | DWT_INT_RXPTO_BIT_MASK | DWT_INT_RXPHE_BIT_MASK | DWT_INT_RXFCE_BIT_MASK | DWT_INT_RXFSL_BIT_MASK | DWT_INT_RXSTO_BIT_MASK,
                     0, DWT_ENABLE_INT_ONLY);

    /*Clearing the SPI ready interrupt*/
    dwt_writesysstatuslo(DWT_INT_RCINIT_BIT_MASK | DWT_INT_SPIRDY_BIT_MASK);

    /* Apply default antenna delay value. See NOTE 2 below. */
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);

    /* Set expected response's delay and timeout.
     * All the devices in the system can immediately start listening */
    dwt_setrxaftertxdelay(0);
    /* Disable the rx timeout*/
    dwt_setrxtimeout(0);

    /*Testing of tx power*/
    dwt_write_reg(TX_POWER_ID, 0x25466788);

    dwt_configciadiag(DW_CIA_DIAG_LOG_ALL);

    /*Sleep Config*/
    dwt_configuresleep(DWT_CONFIG | DWT_PGFCAL, DWT_WAKE_CSN | DWT_SLP_EN | DWT_PRES_SLEEP);

    /*Allow the DW1000 to go to sleep*/
    asUWBSleep();

    /*Set the source address and destination address for some message buffers which do not change*/
    lib_settings->getNodeAddress(&deviceAddress);
    addrPtr = (uint32_t *)&uwbTXMessageBuffer[UWB_MSG_IDX_SRC_ADDRESS];
    *addrPtr = deviceAddress;
}

/*Wakeup is split into 2 stages*/
void uwbWakeUpSt1()
{
    port_set_dw1000_fastrate();
    nrf_gpio_pin_clear(DW_CS_PIN);
}
/*Wakeup is split into 2 stages*/
void uwbWakeUpSt2()
{
    // asSPIInit();
    nrf_gpio_pin_set(DW_CS_PIN);
    // dwt_writesysstatuslo(DWT_INT_RCINIT_BIT_MASK | DWT_INT_SPIRDY_BIT_MASK);
}

void uwbEnableRX()
{

    // For Debug
    // nrf_gpio_pin_set(SIGNAL_A_PIN);
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

bool uwbDisableRX()
{
    if (DWT_DW3000_DEV_ID != (int)dwt_readdevid()) // MP IC ONLY (i.e. DW1000) FOR THIS CODE
    {
        LOG(LVL_INFO, "UWB Error");
        return false;
    }

    dwt_forcetrxoff();
    // nrf_gpio_pin_clear(SIGNAL_A_PIN);
    return true;
}

/*Transmit RST*/
void uwbTransmitMsg(uint8_t function, uint8_t sequence, uint32_t dstAddress, uint32_t nextRS)
{
    uint32_t *dstAddressPtr;
    uint32_t *nextRSTickPtr;

    static uint8_t internalSequence = 0; // Only used during a discovery
    // For Debug
    // nrf_gpio_pin_set(SIGNAL_A_PIN);

    currentFunction = function;

    uwbTXMessageBuffer[UWB_MSG_IDX_FUNCTION_CODE] = function;
    uwbTXMessageBuffer[UWB_MSG_IDX_DETECTED_NBOR_ANCHORS] = nborManagerGetAvailableAnchors();
    switch (function)
    {
    case UWB_FUNCTION_CODE_DISCOVERY:
    case UWB_FUNCTION_CODE_RST:
        uwbTXMessageBuffer[UWB_MSG_IDX_SEQUENCE] = internalSequence;
        break;
    case UWB_FUNCTION_CODE_RSS:
    case UWB_FUNCTION_CODE_RSE:
        uwbTXMessageBuffer[UWB_MSG_IDX_SEQUENCE] = sequence;
        break;
    }

    // Indicate to the timing system that the last message sent was this function
    // uwbControl.currentFunction = function;

    dstAddressPtr = (uint32_t *)&uwbTXMessageBuffer[UWB_MSG_IDX_DST_ADDRESS];
    *dstAddressPtr = dstAddress;

    nextRSTickPtr = (uint32_t *)&uwbTXMessageBuffer[UWB_MSG_IDX_RS_TICKS];
    *nextRSTickPtr = nextRS;

    dwt_write_reg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

    dwt_writetxdata(sizeof(uwbTXMessageBuffer), uwbTXMessageBuffer, 0); /* Zero offset in TX buffer. */
    dwt_writetxfctrl(sizeof(uwbTXMessageBuffer), 0, 1);                 /* Zero offset in TX buffer, ranging. */

    /* Start transmission*/
    if (dwt_starttx(DWT_START_TX_IMMEDIATE) == DWT_ERROR)
    {
        LOG(LVL_INFO, "TX Error");
    }
}

// uwbTransmitMsgCertification(UWB_FUNCTION_CODE_DISCOVERY, 0xAA,0x01020304,0x01020304)

void uwbTransmitMsgCertification(uint8_t function, uint8_t sequence, uint32_t dstAddress, uint32_t nextRS)
{
    uint32_t *dstAddressPtr;
    uint32_t *nextRSTickPtr;

    static uint8_t internalSequence = 0; // Only used during a discovery
    // For Debug
    // nrf_gpio_pin_set(SIGNAL_A_PIN);

    uwbTXMessageBuffer[UWB_MSG_IDX_FUNCTION_CODE] = function;
    switch (function)
    {
    case UWB_FUNCTION_CODE_DISCOVERY:
    case UWB_FUNCTION_CODE_RST:
        uwbTXMessageBuffer[UWB_MSG_IDX_SEQUENCE] = internalSequence;
        break;
    case UWB_FUNCTION_CODE_RSS:
    case UWB_FUNCTION_CODE_RSE:
        uwbTXMessageBuffer[UWB_MSG_IDX_SEQUENCE] = sequence;
        break;
    }

    // Indicate to the timing system that the last message sent was this function
    // uwbControl.currentFunction = function;

    dstAddressPtr = (uint32_t *)&uwbTXMessageBuffer[UWB_MSG_IDX_DST_ADDRESS];
    *dstAddressPtr = dstAddress;

    nextRSTickPtr = (uint32_t *)&uwbTXMessageBuffer[UWB_MSG_IDX_RS_TICKS];
    *nextRSTickPtr = nextRS;

    dwt_write_reg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);

    dwt_writetxdata(sizeof(uwbTXMessageBuffer), uwbTXMessageBuffer, 0); /* Zero offset in TX buffer. */
    dwt_writetxfctrl(sizeof(uwbTXMessageBuffer), 0, 1);                 /* Zero offset in TX buffer, ranging. */

    /* Start transmission*/

    dwt_configcontinuousframemode(50000);
    LOG(LVL_INFO, "CONTINOUS TX ON");
    // if (dwt_starttx(DWT_START_TX_IMMEDIATE) == DWT_ERROR)
    // {
    //     LOG(LVL_INFO, "TX Error");
    // }
}

// This function will verify the device is still in sleep condition
bool asUWBIsSleep()
{
    port_set_dw1000_slowrate();
    int32_t devID = dwt_readdevid();
    LOG(LVL_INFO, "DevID %x", devID);
    return !(devID == DWT_DW3000_DEV_ID);
}

#ifdef APP_LOGIC_DEBUG
void asUWBConfigureDebugGPIO()
{
    nrf_gpio_cfg_output(SIGNAL_A_PIN);
    nrf_gpio_cfg_output(SIGNAL_B_PIN);
    nrf_gpio_pin_clear(SIGNAL_A_PIN);
    nrf_gpio_pin_clear(SIGNAL_B_PIN);
}
#endif