/* Copyright 2026 ASAFE Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    as_battery.c
 * \brief   Battery voltage measurement.
 *
 * Two hardware variants are supported, selected by board.h:
 *
 *  - Direct (default): VDD is the battery. The Wirepas HAL Mcu_voltageGet()
 *    measures VDD internally.
 *
 *  - Divider (BOARD_VBAT_PIN defined): VDD comes from an LDO, and the battery
 *    is read on an analog pin through BOARD_VBAT_R_TOP_KOHM /
 *    BOARD_VBAT_R_BOTTOM_KOHM. Used by asC3W (1M + 1M on P0.05/AIN3).
 *
 * \note The SAADC is driven by register access in single-shot mode, the same
 *       way as the HAL's Mcu_voltageGet(). Both run in application context and
 *       leave the SAADC disabled, so they do not conflict.
 */

#include <stdint.h>

#include "hal_api.h"
#include "board.h"

#include "as_battery.h"

#define DEBUG_LOG_MODULE_NAME "AS BATT"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

static uint16_t lastBatteryMv = 0;

#ifdef BOARD_VBAT_PIN

/* ------------------------------------------------------------------ */
/* Divider measurement on an analog input                             */
/* ------------------------------------------------------------------ */

#if !defined(BOARD_VBAT_R_TOP_KOHM) || !defined(BOARD_VBAT_R_BOTTOM_KOHM)
#error "BOARD_VBAT_PIN requires BOARD_VBAT_R_TOP_KOHM and BOARD_VBAT_R_BOTTOM_KOHM in board.h"
#endif

/* nRF52 GPIO -> SAADC PSELP value (AINn is PSELP n + 1) */
#if BOARD_VBAT_PIN == 2
#define VBAT_PSELP SAADC_CH_PSELP_PSELP_AnalogInput0
#elif BOARD_VBAT_PIN == 3
#define VBAT_PSELP SAADC_CH_PSELP_PSELP_AnalogInput1
#elif BOARD_VBAT_PIN == 4
#define VBAT_PSELP SAADC_CH_PSELP_PSELP_AnalogInput2
#elif BOARD_VBAT_PIN == 5
#define VBAT_PSELP SAADC_CH_PSELP_PSELP_AnalogInput3
#elif BOARD_VBAT_PIN == 28
#define VBAT_PSELP SAADC_CH_PSELP_PSELP_AnalogInput4
#elif BOARD_VBAT_PIN == 29
#define VBAT_PSELP SAADC_CH_PSELP_PSELP_AnalogInput5
#elif BOARD_VBAT_PIN == 30
#define VBAT_PSELP SAADC_CH_PSELP_PSELP_AnalogInput6
#elif BOARD_VBAT_PIN == 31
#define VBAT_PSELP SAADC_CH_PSELP_PSELP_AnalogInput7
#else
#error "BOARD_VBAT_PIN is not an nRF52 analog input (P0.02-P0.05, P0.28-P0.31)"
#endif

#define VBAT_SAADC_CHANNEL 0U
#define VBAT_INT_VREF_MV 600U  /* internal reference */
#define VBAT_INV_GAIN 4U       /* gain 1/4 -> 2.4 V full scale at the pin */
#define VBAT_ADC_MAX 4096U     /* 12-bit single ended */

static bool calibrated = false;

static void saadcRunTask(volatile uint32_t *task, volatile uint32_t *event)
{
    *event = 0;
    *task = 1;
    while (*event == 0)
    {
        /* Wait for the SAADC */
    }
    *event = 0;
}

static void saadcCalibrate(void)
{
    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);
    saadcRunTask(&NRF_SAADC->TASKS_CALIBRATEOFFSET, &NRF_SAADC->EVENTS_CALIBRATEDONE);
    saadcRunTask(&NRF_SAADC->TASKS_STOP, &NRF_SAADC->EVENTS_STOPPED);
    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos);
    calibrated = true;
}

static uint16_t measureDividerMv(void)
{
    volatile int16_t adcResult = 0; /* written by EasyDMA */

    if (!calibrated)
    {
        saadcCalibrate();
    }

    /* Single-shot mode needs exactly one channel connected */
    for (uint8_t i = 0; i < SAADC_CH_NUM; i++)
    {
        NRF_SAADC->CH[i].PSELP = (SAADC_CH_PSELP_PSELP_NC << SAADC_CH_PSELP_PSELP_Pos);
    }

    NRF_SAADC->RESULT.PTR = (uint32_t)&adcResult;
    NRF_SAADC->RESULT.MAXCNT = 1;
    NRF_SAADC->RESOLUTION = (SAADC_RESOLUTION_VAL_12bit << SAADC_RESOLUTION_VAL_Pos);
    /* 8x oversampling, all taken from one SAMPLE task thanks to BURST */
    NRF_SAADC->OVERSAMPLE = (SAADC_OVERSAMPLE_OVERSAMPLE_Over8x << SAADC_OVERSAMPLE_OVERSAMPLE_Pos);

    /* 40 us acquisition: needed for the ~500k source impedance of a 1M/1M divider */
    NRF_SAADC->CH[VBAT_SAADC_CHANNEL].CONFIG =
        (SAADC_CH_CONFIG_MODE_SE << SAADC_CH_CONFIG_MODE_Pos) |
        (SAADC_CH_CONFIG_TACQ_40us << SAADC_CH_CONFIG_TACQ_Pos) |
        (SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_REFSEL_Pos) |
        (SAADC_CH_CONFIG_GAIN_Gain1_4 << SAADC_CH_CONFIG_GAIN_Pos) |
        (SAADC_CH_CONFIG_BURST_Enabled << SAADC_CH_CONFIG_BURST_Pos) |
        (SAADC_CH_CONFIG_RESN_Bypass << SAADC_CH_CONFIG_RESN_Pos) |
        (SAADC_CH_CONFIG_RESP_Bypass << SAADC_CH_CONFIG_RESP_Pos);
    NRF_SAADC->CH[VBAT_SAADC_CHANNEL].PSELN = (SAADC_CH_PSELN_PSELN_NC << SAADC_CH_PSELN_PSELN_Pos);
    NRF_SAADC->CH[VBAT_SAADC_CHANNEL].PSELP = (VBAT_PSELP << SAADC_CH_PSELP_PSELP_Pos);

    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);
    saadcRunTask(&NRF_SAADC->TASKS_START, &NRF_SAADC->EVENTS_STARTED);
    saadcRunTask(&NRF_SAADC->TASKS_SAMPLE, &NRF_SAADC->EVENTS_END);
    saadcRunTask(&NRF_SAADC->TASKS_STOP, &NRF_SAADC->EVENTS_STOPPED);

    /* Release the pin and power down the SAADC */
    NRF_SAADC->CH[VBAT_SAADC_CHANNEL].PSELP = (SAADC_CH_PSELP_PSELP_NC << SAADC_CH_PSELP_PSELP_Pos);
    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos);

    /* Small negative readings are possible near 0 V in SE mode */
    int32_t raw = (adcResult < 0) ? 0 : adcResult;

    /* Voltage at the pin, then scale back up through the divider */
    uint32_t pinMv = ((uint32_t)raw * VBAT_INT_VREF_MV * VBAT_INV_GAIN) / VBAT_ADC_MAX;
    uint32_t vbatMv = (pinMv * (BOARD_VBAT_R_TOP_KOHM + BOARD_VBAT_R_BOTTOM_KOHM)) / BOARD_VBAT_R_BOTTOM_KOHM;

    return (vbatMv > UINT16_MAX) ? UINT16_MAX : (uint16_t)vbatMv;
}

void asBatteryInit(void)
{
    saadcCalibrate();
    LOG(LVL_INFO, "Battery via divider on P0.%u (%u k / %u k)", BOARD_VBAT_PIN, BOARD_VBAT_R_TOP_KOHM, BOARD_VBAT_R_BOTTOM_KOHM);
}

uint16_t asBatteryGetMv(void)
{
    lastBatteryMv = measureDividerMv();
    return lastBatteryMv;
}

#else /* BOARD_VBAT_PIN */

/* ------------------------------------------------------------------ */
/* Direct: VDD is the battery                                         */
/* ------------------------------------------------------------------ */

void asBatteryInit(void)
{
    Mcu_voltageInit();
    LOG(LVL_INFO, "Battery measured on VDD");
}

uint16_t asBatteryGetMv(void)
{
    lastBatteryMv = Mcu_voltageGet();
    return lastBatteryMv;
}

#endif /* BOARD_VBAT_PIN */

uint16_t asBatteryGetLastMv(void)
{
    return lastBatteryMv;
}
