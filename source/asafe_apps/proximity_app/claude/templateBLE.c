/*
 * Bare-metal Eddystone-UID beacon for nRF52832 (no SoftDevice).
 *
 *  - ADV_NONCONN_IND (non-connectable, non-scannable)
 *  - Channels 37, 38, 39 every advertising event
 *  - ~3 Hz advertising rate (333 ms average, incl. BLE-mandated random advDelay)
 *  - RTC0 on LFCLK for timing, CPU sleeps (WFE) between events
 *  - HFXO only running during the ~1.5 ms radio burst
 *
 * AdvA (random address) carries device info, bytes in on-air order:
 *   [0..3] serial number, MSB first
 *   [4]    mode (0 = INVENTORY, 1 = RUN)
 *   [5]    device type (0..255)
 *
 * Build with the Nordic MDK (nrf.h, gcc_startup_nrf52.S, system_nrf52.c,
 * linker script) and -DNRF52832_XXAA. system_nrf52.c applies the chip errata
 * workarounds in SystemInit(), so keep it in the build.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nrf.h"

/* ------------------------------------------------------------------ */
/* Configuration                                                       */
/* ------------------------------------------------------------------ */

/* 1 = 32.768 kHz crystal fitted, 0 = use internal RC oscillator.      */
#define USE_LFXO 1

/* 1 = board has the DC/DC inductor fitted (saves power), else 0.      */
#define USE_DCDC 0

/* Radio TX power. */
#define RADIO_TX_POWER RADIO_TXPOWER_TXPOWER_0dBm

/* Eddystone "Ranging Data": calibrated TX power at 0 m, in dBm (0xEC). */
#define EDDYSTONE_TX_AT_0M (-20)

/* Timing (RTC ticks @ 32768 Hz).
 * BLE requires advInterval + advDelay, with advDelay random in 0..10 ms.
 * 10759 ticks = 328.3 ms, plus mean advDelay of ~5 ms  =>  ~333 ms  =>  3 Hz. */
#define ADV_INTERVAL_TICKS 10759u
#define ADV_DELAY_MAX_TICKS 328u /* exclusive upper bound, ~10 ms */

/* Device identity used at startup. */
#define DEVICE_SERIAL 304327355u /* 0x1223AABB */
#define DEVICE_TYPE 35u

/* Beacon mode values */
#define BEACON_MODE_INVENTORY ((uint8_t)0u)
#define BEACON_MODE_RUN ((uint8_t)1u)

/* Namespace: SHA-1 of www.redlore.com */
static const uint8_t eddystone_namespace[10] = {
    0xD5, 0x47, 0xA3, 0xF9, 0xE5, 0xB5, 0xC0, 0xCC, 0x97, 0x79};

/* Instance: generated from the network ID */
static uint8_t eddystone_instance[6] = {
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
static uint8_t adv_pdu[2 + MAX_ADV_PAYLOAD];

static volatile bool adv_due = false;

/* ------------------------------------------------------------------ */
/* Packet construction                                                 */
/* ------------------------------------------------------------------ */

/*
 * Builds the advertising PDU.
 * e.g. serial 304327355 (0x1223AABB), INVENTORY, type 35
 *   -> AdvA bytes on air: 12 23 AA BB 00 23
 *
 * Only call this between advertising events (from the main loop),
 * never while the radio is transmitting, since the radio reads
 * adv_pdu by DMA.
 */
static void build_adv_pdu(uint32_t serial_number,
                          uint8_t mode,
                          uint8_t device_type)
{
    uint8_t *p = adv_pdu;

    *p++ = PDU_TYPE_ADV_NONCONN | PDU_HDR_TXADD_RANDOM; /* 0x42         */
    uint8_t *len = p++;                                 /* LENGTH       */

    /* AdvA */
    *p++ = (uint8_t)(serial_number >> 24);
    *p++ = (uint8_t)(serial_number >> 16);
    *p++ = (uint8_t)(serial_number >> 8);
    *p++ = (uint8_t)(serial_number);
    *p++ = mode;
    *p++ = device_type;

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
    memcpy(p, eddystone_namespace, 10);
    p += 10;
    memcpy(p, eddystone_instance, 6);
    p += 6;

    *len = (uint8_t)(p - &adv_pdu[2]); /* = 35 */
}

/* ------------------------------------------------------------------ */
/* Clocks                                                              */
/* ------------------------------------------------------------------ */

static void lfclk_start(void)
{
#if USE_LFXO
    NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos;
#else
    NRF_CLOCK->LFCLKSRC = CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos;
#endif
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
    }
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
}

/* The RADIO needs the HF crystal; run it only while transmitting. */
static void hfxo_start(void)
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
    {
    }
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
}

static void hfxo_stop(void)
{
    NRF_CLOCK->TASKS_HFCLKSTOP = 1;
}

/* ------------------------------------------------------------------ */
/* RNG for advDelay                                                    */
/* ------------------------------------------------------------------ */

static uint8_t rng_byte(void)
{
    NRF_RNG->EVENTS_VALRDY = 0;
    NRF_RNG->TASKS_START = 1;
    while (NRF_RNG->EVENTS_VALRDY == 0)
    {
    }
    NRF_RNG->TASKS_STOP = 1;
    return (uint8_t)NRF_RNG->VALUE;
}

static uint32_t random_adv_delay(void)
{
    uint16_t r = ((uint16_t)rng_byte() << 8) | rng_byte();
    return r % ADV_DELAY_MAX_TICKS;
}

/* ------------------------------------------------------------------ */
/* Radio                                                               */
/* ------------------------------------------------------------------ */

static void radio_init(void)
{
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

    NRF_RADIO->PACKETPTR = (uint32_t)adv_pdu;

    /* TXEN -> READY -> START -> END -> DISABLE -> DISABLED, no CPU needed */
    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk |
                        RADIO_SHORTS_END_DISABLE_Msk;
}

static void radio_send_on_channel(uint8_t freq_offset, uint8_t ch_index)
{
    NRF_RADIO->FREQUENCY = freq_offset; /* MHz above 2400 */
    NRF_RADIO->DATAWHITEIV = ch_index;  /* whitening seeded by channel index */

    NRF_RADIO->EVENTS_DISABLED = 0;
    NRF_RADIO->TASKS_TXEN = 1;
    while (NRF_RADIO->EVENTS_DISABLED == 0)
    {
    }
    NRF_RADIO->EVENTS_DISABLED = 0;
}

static void advertising_event(void)
{
    radio_send_on_channel(2, 37);  /* 2402 MHz */
    radio_send_on_channel(26, 38); /* 2426 MHz */
    radio_send_on_channel(80, 39); /* 2480 MHz */
}

/* ------------------------------------------------------------------ */
/* RTC0 scheduler                                                      */
/* ------------------------------------------------------------------ */

static void rtc_init(void)
{
    NRF_RTC0->TASKS_STOP = 1;
    NRF_RTC0->TASKS_CLEAR = 1;
    NRF_RTC0->PRESCALER = 0; /* 32768 Hz tick */
    NRF_RTC0->CC[0] = ADV_INTERVAL_TICKS;
    NRF_RTC0->EVENTS_COMPARE[0] = 0;
    NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE0_Msk;

    NVIC_ClearPendingIRQ(RTC0_IRQn);
    NVIC_SetPriority(RTC0_IRQn, 3);
    NVIC_EnableIRQ(RTC0_IRQn);

    NRF_RTC0->TASKS_START = 1;
}

void RTC0_IRQHandler(void)
{
    if (NRF_RTC0->EVENTS_COMPARE[0])
    {
        NRF_RTC0->EVENTS_COMPARE[0] = 0;
        (void)NRF_RTC0->EVENTS_COMPARE[0]; /* flush write before exit */

        /* Schedule next event: interval + random advDelay (24-bit counter wrap) */
        NRF_RTC0->CC[0] = (NRF_RTC0->CC[0] + ADV_INTERVAL_TICKS + random_adv_delay()) & RTC_COUNTER_COUNTER_Msk;

        adv_due = true;
    }
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

int main(void)
{
#if USE_DCDC
    NRF_POWER->DCDCEN = 1;
#endif

    build_adv_pdu(DEVICE_SERIAL, BEACON_MODE_INVENTORY, DEVICE_TYPE);
    lfclk_start();
    radio_init();
    rtc_init();

    for (;;)
    {
        while (!adv_due)
        {
            __WFE(); /* sleep until RTC IRQ */
        }
        adv_due = false;

        /* To change mode/serial/type at runtime, call build_adv_pdu()
         * here, before the radio burst, e.g.:
         *   build_adv_pdu(DEVICE_SERIAL, BEACON_MODE_RUN, DEVICE_TYPE);
         */

        hfxo_start();
        advertising_event();
        hfxo_stop();
    }
}