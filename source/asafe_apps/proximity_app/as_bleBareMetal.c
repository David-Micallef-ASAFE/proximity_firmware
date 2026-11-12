/* Copyright 2026 Redlore. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_bleBareMetal.c
 *
 * Bare-metal Eddystone-UID beacon for nRF52832, driving the RADIO directly.
 *
 *  - ADV_NONCONN_IND (non-connectable, non-scannable)
 *  - Channels 37, 38, 39 every advertising event
 *  - ~3 Hz advertising rate (333 ms average, incl. BLE-mandated random advDelay)
 *  - Timing busy-waits on the DWT cycle counter (64 MHz CPU clock), no
 *    interrupts or timer peripherals used
 *  - HFXO kept running for the whole session (simple, not low power)
 *
 * The Wirepas stack must be stopped: it owns the RADIO when running.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "api.h"
#include "mcu.h"

#include "hal_api.h"
#include "local_provisioning.h"
#include "as_bleBareMetal.h"
#include "as_battery.h"
#include "board.h"
#include "as_types.h"
#ifdef APP_BUZZER
#include "as_buzzer.h"
#endif

#define DEBUG_LOG_MODULE_NAME "BLE BM"
/** To activate logs, configure the following line with "LVL_INFO". */
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

/* ------------------------------------------------------------------ */
/* Configuration                                                       */
/* ------------------------------------------------------------------ */

/* Radio TX power. */
#define RADIO_TX_POWER RADIO_TXPOWER_TXPOWER_0dBm

/* Eddystone "Ranging Data": calibrated TX power at 0 m, in dBm (0xEC). */
#define EDDYSTONE_TX_AT_0M (-20)

/* Timing (microseconds).
 * BLE requires advInterval + advDelay, with advDelay random in 0..10 ms.
 * 328.3 ms, plus mean advDelay of ~5 ms  =>  ~333 ms  =>  3 Hz. */
#define CPU_CYCLES_PER_US 64u /* nRF52832 runs at 64 MHz */
#define ADV_INTERVAL_US 100000u
#define ADV_DELAY_MAX_US 10000u /* exclusive upper bound */

/* Namespace: first 10 bytes of SHA-1 of www.asafe.com
 * (was www.redlore.com: D5 47 A3 F9 E5 B5 C0 CC 97 79) */
static const uint8_t eddystoneNamespace[10] = {
    0x0C, 0x17, 0xAF, 0xFC, 0x77, 0x8D, 0x36, 0x17, 0xD8, 0x53};

/* Instance (6 bytes):
 *   [0..1] battery voltage in mV, big-endian (e.g. 3012 mV -> 0B C4)
 *   [2]    flags, see INSTANCE_FLAG_*
 *   [3..5] reserved, 0x00 */
#define INSTANCE_VOLTAGE_IDX 0
#define INSTANCE_FLAGS_IDX 2

/* Instance flags (bit masks) */
#define INSTANCE_FLAG_PROVISIONED 0x01u /* Local_provisioning_is_provisioned() */

static uint8_t eddystoneInstance[6] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* ------------------------------------------------------------------ */
/* BLE constants                                                       */
/* ------------------------------------------------------------------ */

#define BLE_ADV_ACCESS_ADDR 0x8E89BED6u
#define BLE_ADV_CRC_INIT 0x555555u
#define BLE_CRC_POLY 0x00065Bu

#define PDU_TYPE_ADV_NONCONN 0x02u
#define PDU_HDR_TXADD_RANDOM 0x40u

#define MAX_ADV_PAYLOAD 37u /* AdvA (6) + AdvData (31) */

/* RAM layout with S0LEN=1, LFLEN=8, S1LEN=0: [S0 header][LENGTH][payload] */
static uint8_t advPdu[2 + MAX_ADV_PAYLOAD];

/* ------------------------------------------------------------------ */
/* Packet construction                                                 */
/* ------------------------------------------------------------------ */

/* Battery voltage in mV (as_battery handles direct VDD vs divider boards) */
static uint16_t getVoltage(void)
{
    return asBatteryGetMv();
}

/* Device state flags carried in the instance flags byte */
static uint8_t getFlags(void)
{
    uint8_t flags = 0;

    if (Local_provisioning_is_provisioned())
    {
        flags |= INSTANCE_FLAG_PROVISIONED;
    }
    return flags;
}

/*
 * Builds the advertising PDU.
 * e.g. serial 304327355 (0x1223AABB), INVENTORY, type 35
 *   -> AdvA bytes on air: 23 00 BB AA 23 12
 *   -> shown by scanners: 12:23:AA:BB:00:23
 *
 * Never call while the radio is transmitting, since the radio reads
 * advPdu by DMA.
 */
static void buildAdvPdu(uint32_t serialNumber, uint8_t mode, uint8_t deviceType, uint16_t voltageMv, uint8_t flags)
{
    uint8_t *p = advPdu;

    eddystoneInstance[INSTANCE_VOLTAGE_IDX] = (uint8_t)(voltageMv >> 8);
    eddystoneInstance[INSTANCE_VOLTAGE_IDX + 1] = (uint8_t)(voltageMv);
    eddystoneInstance[INSTANCE_FLAGS_IDX] = flags;

    *p++ = PDU_TYPE_ADV_NONCONN | PDU_HDR_TXADD_RANDOM; /* 0x42         */
    uint8_t *len = p++;                                 /* LENGTH       */

    /* AdvA - sent LSB first, so scanners display it as
     * [serial MSB..LSB]:[mode]:[deviceType] */
    *p++ = deviceType;
    *p++ = mode;
    *p++ = (uint8_t)(serialNumber);
    *p++ = (uint8_t)(serialNumber >> 8);
    *p++ = (uint8_t)(serialNumber >> 16);
    *p++ = (uint8_t)(serialNumber >> 24);

    /* AD: Flags - BR/EDR not supported (beacon only) */
    *p++ = 0x02;
    *p++ = 0x01;
    *p++ = 0x04;

    /* AD: Complete list of 16-bit UUIDs - Eddystone 0xFEAA */
    *p++ = 0x03;
    *p++ = 0x03;
    *p++ = 0xAA;
    *p++ = 0xFE;

    /* AD: Service Data - Eddystone-UID */
    *p++ = 0x15; /* length (21)                    */
    *p++ = 0x16; /* Service Data - 16-bit UUID     */
    *p++ = 0xAA;
    *p++ = 0xFE; /* Eddystone UUID                 */
    *p++ = 0x00; /* Frame type: UID                */
    *p++ = (uint8_t)(int8_t)EDDYSTONE_TX_AT_0M;
    memcpy(p, eddystoneNamespace, 10);
    p += 10;
    memcpy(p, eddystoneInstance, 6);
    p += 6;

    *len = (uint8_t)(p - &advPdu[2]); /* = 35 */
}

/* ------------------------------------------------------------------ */
/* Clocks                                                              */
/* ------------------------------------------------------------------ */

/* The RADIO needs the HF crystal. */
static void hfxoStart(void)
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
    {
    }
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
}

static void hfxoStop(void)
{
    NRF_CLOCK->TASKS_HFCLKSTOP = 1;
}

/* ------------------------------------------------------------------ */
/* RNG for advDelay                                                    */
/* ------------------------------------------------------------------ */

static uint8_t rngByte(void)
{
    NRF_RNG->EVENTS_VALRDY = 0;
    NRF_RNG->TASKS_START = 1;
    while (NRF_RNG->EVENTS_VALRDY == 0)
    {
    }
    NRF_RNG->TASKS_STOP = 1;
    return (uint8_t)NRF_RNG->VALUE;
}

static uint32_t randomAdvDelay(void)
{
    uint16_t r = ((uint16_t)rngByte() << 8) | rngByte();
    return r % ADV_DELAY_MAX_US;
}

/* ------------------------------------------------------------------ */
/* Cycle counter delay                                                 */
/* ------------------------------------------------------------------ */

static void cycleCounterInit(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* ------------------------------------------------------------------ */
/* Heartbeat indicator (buzzer beep, else LED flash)                   */
/* ------------------------------------------------------------------ */

/* The as_buzzer.c / as_led.c sequences run from the app scheduler, which
 * doesn't run while we block, so the indicator is driven directly here. */
#if defined(APP_BUZZER)
#define INDICATOR_ENABLED
#define INDICATOR_ON_US 50000u /* same as asBuzzerBeepSuperShort() */

static void indicatorOn(void)
{
    asBuzzerToneOn();
}

static void indicatorOff(void)
{
    asBuzzerToneOff();
}

#elif defined(APP_LED)
#define INDICATOR_ENABLED
#define INDICATOR_ON_US 100000u /* same as as_led.c flash */

/* LED 0, except K4W which uses LED 1 */
#if ASAFE_DEVICE_TYPE == DEVICE_TYPE_KKM_K4W
#define INDICATOR_LED_IDX 1
#else
#define INDICATOR_LED_IDX 0
#endif

static const uint8_t ledPins[] = BOARD_LED_PIN_LIST;
#define INDICATOR_LED_PIN (ledPins[INDICATOR_LED_IDX])

static void indicatorOn(void)
{
    if (BOARD_LED_ACTIVE_LOW)
    {
        nrf_gpio_pin_clear(INDICATOR_LED_PIN);
    }
    else
    {
        nrf_gpio_pin_set(INDICATOR_LED_PIN);
    }
}

static void indicatorOff(void)
{
    if (BOARD_LED_ACTIVE_LOW)
    {
        nrf_gpio_pin_set(INDICATOR_LED_PIN);
    }
    else
    {
        nrf_gpio_pin_clear(INDICATOR_LED_PIN);
    }
}
#endif

#ifdef INDICATOR_ENABLED
#define INDICATOR_PERIOD_US 1000000u

static uint32_t indicatorStart;
static bool indicatorIsOn;

static void indicatorStartNow(void)
{
#if !defined(APP_BUZZER)
    nrf_gpio_cfg_output(INDICATOR_LED_PIN);
#endif
    indicatorStart = DWT->CYCCNT;
    indicatorOn();
    indicatorIsOn = true;
}

/* Called from the busy-wait: turns the indicator off after INDICATOR_ON_US
 * and starts the next one every INDICATOR_PERIOD_US */
static void indicatorService(void)
{
    uint32_t elapsed = DWT->CYCCNT - indicatorStart;

    if (indicatorIsOn && elapsed >= INDICATOR_ON_US * CPU_CYCLES_PER_US)
    {
        indicatorOff();
        indicatorIsOn = false;
    }
    if (elapsed >= INDICATOR_PERIOD_US * CPU_CYCLES_PER_US)
    {
        indicatorStart += INDICATOR_PERIOD_US * CPU_CYCLES_PER_US;
        indicatorOn();
        indicatorIsOn = true;
    }
}

static void indicatorStop(void)
{
    indicatorOff();
    indicatorIsOn = false;
}
#endif

/* Busy-wait until delayUs has passed since startCycles. Unsigned subtraction
 * handles CYCCNT wrap (every ~67 s), as long as delayUs is well below that. */
static void waitUntil(uint32_t startCycles, uint32_t delayUs)
{
    uint32_t delayCycles = delayUs * CPU_CYCLES_PER_US;
    while ((DWT->CYCCNT - startCycles) < delayCycles)
    {
#ifdef INDICATOR_ENABLED
        indicatorService();
#endif
    }
}

/* ------------------------------------------------------------------ */
/* Radio                                                               */
/* ------------------------------------------------------------------ */

static void radioInit(void)
{
    /* Power cycle to reset any configuration left behind by the stack */
    NRF_RADIO->POWER = 0;
    NRF_RADIO->POWER = 1;

    NRF_RADIO->MODE = RADIO_MODE_MODE_Ble_1Mbit << RADIO_MODE_MODE_Pos;
    NRF_RADIO->MODECNF0 = RADIO_MODECNF0_RU_Fast << RADIO_MODECNF0_RU_Pos; /* 40 us ramp-up */
    NRF_RADIO->TXPOWER = RADIO_TX_POWER << RADIO_TXPOWER_TXPOWER_Pos;

    /* S0 = 1 byte (PDU header), LENGTH = 8 bits, no S1 */
    NRF_RADIO->PCNF0 = (1u << RADIO_PCNF0_S0LEN_Pos) |
                       (8u << RADIO_PCNF0_LFLEN_Pos) |
                       (0u << RADIO_PCNF0_S1LEN_Pos);

    NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Enabled << RADIO_PCNF1_WHITEEN_Pos) |
                       (RADIO_PCNF1_ENDIAN_Little << RADIO_PCNF1_ENDIAN_Pos) |
                       (3u << RADIO_PCNF1_BALEN_Pos) |
                       (0u << RADIO_PCNF1_STATLEN_Pos) |
                       (MAX_ADV_PAYLOAD << RADIO_PCNF1_MAXLEN_Pos);

    /* Access address 0x8E89BED6: prefix = 0x8E, base = 0x89BED6 */
    NRF_RADIO->PREFIX0 = (BLE_ADV_ACCESS_ADDR >> 24) & 0xFFu;
    NRF_RADIO->BASE0 = (BLE_ADV_ACCESS_ADDR << 8);
    NRF_RADIO->TXADDRESS = 0;
    NRF_RADIO->RXADDRESSES = 1;

    NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Three << RADIO_CRCCNF_LEN_Pos) |
                        (RADIO_CRCCNF_SKIPADDR_Skip << RADIO_CRCCNF_SKIPADDR_Pos);
    NRF_RADIO->CRCPOLY = BLE_CRC_POLY;
    NRF_RADIO->CRCINIT = BLE_ADV_CRC_INIT;

    NRF_RADIO->PACKETPTR = (uint32_t)advPdu;

    /* TXEN -> READY -> START -> END -> DISABLE -> DISABLED, no CPU needed */
    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk |
                        RADIO_SHORTS_END_DISABLE_Msk;
}

/* Return the radio to its reset state so the stack can take it back later */
static void radioDeinit(void)
{
    NRF_RADIO->SHORTS = 0;
    NRF_RADIO->POWER = 0;
    NRF_RADIO->POWER = 1;
}

static void radioSendOnChannel(uint8_t freqOffset, uint8_t chIndex)
{
    NRF_RADIO->FREQUENCY = freqOffset; /* MHz above 2400 */
    NRF_RADIO->DATAWHITEIV = chIndex;  /* whitening seeded by channel index */

    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_TXEN = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0)
    {
    }
    NRF_RADIO->EVENTS_DISABLED = 0;
}

static void advertisingEvent(void)
{
    LOG(LVL_INFO, "ADV");
    radioSendOnChannel(2, 37);  /* 2402 MHz */
    radioSendOnChannel(26, 38); /* 2426 MHz */
    radioSendOnChannel(80, 39); /* 2480 MHz */
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

bool asBLEBareMetalStart(uint32_t timeSeconds, uint32_t serialNumber, asMode_e mode, uint8_t deviceType,
                         asBLEBareMetalDoneCb_f doneCb)
{
    uint64_t remainingUs = (uint64_t)timeSeconds * 1000000u;

    if (remainingUs == 0)
    {
        return false;
    }

    if (lib_state->getStackState() == APP_LIB_STATE_STARTED)
    {
        LOG(LVL_ERROR, "Wirepas stack running, bare-metal BLE not started");
        return false;
    }

    LOG(LVL_INFO, "BLE bare-metal start: %u s, serial %u, mode %u, type %u",
        timeSeconds, serialNumber, (uint8_t)mode, deviceType);

    /* Measured once, before the radio and buzzer load the battery */
    uint16_t voltageMv = getVoltage();
    LOG(LVL_INFO, "Battery %u mV", voltageMv);

    uint8_t flags = getFlags();
    LOG(LVL_INFO, "Flags 0x%02x", flags);

    buildAdvPdu(serialNumber, (uint8_t)mode, deviceType, voltageMv, flags);
    cycleCounterInit();
    hfxoStart();
    radioInit();
#ifdef INDICATOR_ENABLED
    indicatorStartNow();
#endif

    for (;;)
    {
        uint32_t eventStart = DWT->CYCCNT;
        advertisingEvent();

        /* Next event: interval + random advDelay, measured from this event's start */
        uint32_t stepUs = ADV_INTERVAL_US + randomAdvDelay();
        if (stepUs >= remainingUs)
        {
            break;
        }
        remainingUs -= stepUs;

        waitUntil(eventStart, stepUs);
    }

#ifdef INDICATOR_ENABLED
    indicatorStop();
#endif
    radioDeinit();
    hfxoStop();

    LOG(LVL_INFO, "BLE bare-metal stop");

    if (doneCb != NULL)
    {
        doneCb();
    }
    return true;
}
