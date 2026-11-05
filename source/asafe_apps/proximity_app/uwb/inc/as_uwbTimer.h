/* Copyright 2026 ASAFE. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

#ifndef __AS_UWB_TIMER_H_
#define __AS_UWB_TIMER_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"

#define RANDOM_TICK(MAX_TICK) (Random_get32() % MAX_TICK)

#define RTC_COUNTER_MAX 0X00FFFFFF

// #define TIMER_PRESCALER 15 // This will give us a tick of 1/2048 = 488uS
#define TIMER_PRESCALER 0 // This will give us a tick of 1/32768 = 30.5uS

#define TIMER_SLOT_TICK (32 * 50)
#define TIMER_SLOT_US 48828

#define TIMER_100MS_TICK 3277
#define TIMER_1S_TICK 32768
#define TIMER_10S_TICK (TIMER_1S_TICK * 10)
#define TIMER_1MIN_TICK (TIMER_1S_TICK * 60)
#define TIMER_10MIN_TICK (TIMER_1MIN_TICK * 10)
#define TIMER_60MIN_TICK (TIMER_1MIN_TICK * 60)
#define TIMER_1MS_TICK 33
#define TIMER_100US_TICK 3

#define TIMER_CYCLE_TICK TIMER_10S_TICK // This should be aligned to 1 second
#define TIMER_CYCLE_S (TIMER_CYCLE_TICK / TIMER_1S_TICK)

// TX Discovery
#define TIMER_TX_DISCOVERY_PROCESSING TIMER_1MS_TICK // 500uS (tested with logic analyzer)
// Wakeup
#define TIMER_WK1_PROCESSING 80                   // 500uS
#define TIMER_WK2_PROCESSING (TIMER_1MS_TICK * 3) // 3mS
#define TIMER_WAKEUP_PROCESSING (TIMER_WK1_PROCESSING + TIMER_WK2_PROCESSING)
// Ranging
#define TIMER_RANGING_ST1 1200 // 25mS
#define TIMER_RANGING_ST2 (TIMER_1MS_TICK * 5)
// Discovery Nbor Processing
#define TIMER_DISCOVERY_NBOR_PROCESSING (TIMER_1MS_TICK * 10) // 25mS
// Time to Range
#define TIMER_TIME_TO_RANGING (TIMER_DISCOVERY_NBOR_PROCESSING + TIMER_WAKEUP_PROCESSING + (TIMER_100MS_TICK / 2))

// Nbor Discovery
#define TIMER_DISCOVER_NBORS_PROCESSING_ST1 (TIMER_1MS_TICK * 500) // 250mS
#define TIMER_DISCOVER_NBORS_PROCESSING_ST2 (TIMER_1MS_TICK * 10)
#define TIMER_DISCOVER_NBORS_PROCESSING (TIMER_DISCOVER_NBORS_PROCESSING_ST1 + TIMER_DISCOVER_NBORS_PROCESSING_ST2)
// TX Nbor RST
#define TIMER_TX_NBOR_RST_PROCESSING (TIMER_1MS_TICK * 5)
// Sleep Processing
#define TIMER_SLEEP_PROCESSING (TIMER_1MS_TICK * 2)
#define TIMER_SLEEP_PROCESSING_EXTRA TIMER_1MS_TICK
// Min to allow DW to sleep
#define TIMER_MIN_TO_ALLOW_SLEEP (TIMER_WAKEUP_PROCESSING + TIMER_SLEEP_PROCESSING + TIMER_SLEEP_PROCESSING_EXTRA)

/*DWM1000 TIMINGS */
// Processing
#define TIMER_TICK_PROCESSING TIMER_1MS_TICK

// Wakeup
#define TIMER_TICK_WAKEUP_ST1 TIMER_1MS_TICK                          // 1mS
#define TIMER_TICK_WAKEUP_ST2 TIMER_1MS_TICK + (TIMER_100US_TICK * 5) // 1.5mS

// Sleep
#define TIMER_TICK_SLEEP TIMER_1MS_TICK

// Ranging
#define TIMER_TICK_RANGING_RSS TIMER_1MS_TICK
#define TIMER_TICK_RANGING_RSE TIMER_1MS_TICK
#define TIMER_TICK_RANGING (TIMER_TICK_RANGING_RSS + TIMER_TICK_RANGING_RSE)

/*This is used to wait for all DW1000 SPI comms to finish and go back into idle*/
#define TIMER_TICK_IDLE 10
#define TIMER_TICK_IDLE_MIN 5

/*Ranging Slot Timing*/
#define TIMER_CYCLE_PERIOD_TICK TIMER_10S_TICK
#define TIMER_RS_DURATION (TIMER_TICK_WAKEUP + TIMER_TICK_RS_PERIOD + TIMER_TICK_IDLE)

/*TX Discovery Timing*/
#define TIMER_TX_DISCOVERY_PERIOD_TICK (TIMER_1S_TICK / 4)
#define TIMER_TX_DISCOVERY_DURATION (TIMER_TICK_WAKEUP + TIMER_TICK_IDLE)

/*This is the reserved 5% of the slot for the RSS and RSE*/
#define TIMER_RS_RESERVE_TICK (TIMER_1MS_TICK + (TIMER_100US_TICK * 2))

/*This is the period between RSS and RSE transmission (24ms)*/
#define TIMER_TICK_RS_PERIOD (TIMER_1MS_TICK * 24)

/*This is the delay before the UWB operations start to work*/
#define TIMER_INIT_DELAY_TICK (TIMER_1S_TICK * 5)

/*The Nbor discovery period*/
#define TIMER_DISCOVER_NBORS_PERIOD_TICK TIMER_60MIN_TICK
#define TIMER_DISCOVER_NBORS_SAMPLING_PERIOD_TICK TIMER_10MIN_TICK
#define TIMER_DISCOVER_NBORS_DUTY_CYCLE_TICK (TIMER_1MS_TICK * 250)
#define TIMER_DISCOVER_NBORS_DURATION (TIMER_TICK_WAKEUP + TIMER_DISCOVER_NBORS_DUTY_CYCLE_TICK + (TIMER_TICK_IDLE * 2))

#define TIMER_TICK_WAKEUP (TIMER_TICK_WAKEUP_ST1 + TIMER_TICK_WAKEUP_ST2)

/*We do not execute a task if its less than the pre task tick*/
#define TIMER_MIN_PRE_TASK_TICK (TIMER_TICK_WAKEUP + 5)

/*This is the min time the next task should be due in to allow the DW1000 to sleep*/
#define TIMER_MIN_NEXT_TASK_ALLOW_SLEEP (TIMER_MIN_PRE_TASK_TICK + (TIMER_TICK_IDLE * 3))

/*V1 Timings*/
#define TIMER_COARSE_DIVIDER 1024

/*This is the period the SYNC messages are transmitted at from the Master*/
#define TIMER_TICK_SYNC_TRANSMISSIONS 2048
/*This is the time time we will wait to receive a SYNC message - Note this is only used when the device is not synced to the network*/
#define TIMER_TICK_SYNC_LISTEN (TIMER_TICK_SYNC_TRANSMISSIONS + 1)
/*Common Anchor Timing*/
#define TIMER_TICK_SYNC_LISTEN_WAIT 20000 // This is idle time.  we do not want it to sync exactly with the cycle time

/*Total ticks in the full cycle*/
#define TIMER_TICK_CYCLE_PERIOD 20480 // 10 seconds
/*Total ticks in the listening window*/
#define TIMER_TICK_WINDOW 52 // 25.39mS
/*Ticks of hysterisis.  The cycle will be shifted by the ticks below to allow for hysterisis*/
#define TIMER_TICK_WINDOW_HYSTERISIS 50
/*Ticks to wait in init for CA and Tags*/
#define TIMER_TICK_INIT 2048 // This is mostly for debuging and can be as small as 1 tick
#define TIMER_ONE_TICK 1

/*Ticks required to transmit a message - In reality the time used is less*/
#define TIMER_TICK_TRANSMIT 4
/*Ticks required to sleep*/
#define TIMER_TICK_SLEEP__ 1

/*The number of transmissions sent between a cycle*/
#define TIMER_NUM_SYNC_TRANSMISSIONS (TIMER_TICK_CYCLE_PERIOD / TIMER_TICK_SYNC_TRANSMISSIONS) - 1

/*Unused for now*/
// #define TIMER_ANCHOR_LISTEN_TICKS_TIMEOUT 6

// #define TIMER_MISSED_SN1_THRESHOLD_EXTENDED_LISTEN 2
// #define TIMER_MISSED_SN1_THRESHOLD_HUNT 4

// #define TIMER_RX_OVERHEAD_TICKS 2

// #define TIMER_TICK_LISTEN (TIMER_CYCLE_TICKS_TOTAL - (TIMER_TICK_WAKEUP_ST1 + TIMER_TICK_WAKEUP_ST2 + TIMER_TICK_TRANSMIT_SYNC2)) // 1mS

// #define TIMER_TICK_HUNT 220        // We need 100mS
// #define TIMER_TICK_HUNT_SLEEP 2200 // We need 1100mS

// #define TIMER_SYNC_WINDOW_TICKS 52

// #define TIMER_TICK_SLEEP_SYNC1 2048

// #define TIMER_CYCLE_DUTY_TICKS_TS1 20                                                    // This is the time from the first Sync Signal to the second
// #define TIMER_CYCLE_DUTY_TICKS_TS2 (TIMER_CYCLE_DUTY_TICKS - TIMER_CYCLE_DUTY_TICKS_TS1) // This is the time from second sync signal to sleep

// #define TIMER_CYCLE_SLEEP_TICKS (TIMER_CYCLE_PERIOD_TICKS - TIMER_CYCLE_DUTY_TICKS)

// #define TIMER_TICK_PREPARE_SN1 (TIMER_TICK_WAKEUP_ST1 + TIMER_TICK_WAKEUP_ST2)
// #define TIMER_UWB_LISTEN_TIMEOUT_UNSYNC_MS 9
// #define TIMER_UWB_LISTEN_TIMEOUT_SYNC_MS 1

// #define TIMER_SYNC_BEACON_NUM 9

void asTimerInit();
void timerStop();

// void timerSync(syncMessage_t *syncMessage, uint32_t travelTime);

void timerSync(uint8_t syncCount);

void timerTickTest();
app_lib_data_receive_res_e timerWPSync(const app_lib_data_received_t *data);
uint32_t timerGetServingAnchor();

uint32_t timerGetSlotOffsetUs();
int16_t timerGetSlotForNbor(uint16_t nextSlotIn, uint8_t nborSlotIdx);

void asUWBTimerRequestMotionScan();
void asUWBTimerRequestStationaryScan();
void asUWBTimerInidicateCollisionDetected();

#endif /* __AS_UWB_TIMER_H_ */
