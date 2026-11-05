/* Copyright 2019 Redlore. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_buzzer.c
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

#include "board.h"
#include "as_buzzer.h"

/*----------------DEBUG-------------------------------------------*/
#define DEBUG_LOG_MODULE_NAME "OSAT_MOTION"
#define DEBUG_LOG_MAX_LEVEL LVL_DEBUG
#include "debug_log.h"
/*----------------DEBUG-------------------------------------------*/

#define BUZZER_TASK_EXECUTION 500

#define BUZZER_SEQUENCE_TIME_DELAY_MS 500       // used to introduce a delay in the sequence
#define BUZZER_SEQUENCE_TIME_DELAY_SHORT_MS 100 // used to introduce a delay in the sequence
#define BUZZER_SEQUENCE_TIME_DELAY_LONG_MS 800  // used to introduce a delay in the sequence

#define PWM_PERIOD 8000 // 1ms
#define PWM_CH0_DUTY 1000

#define BUZZER_ACTION_TIMEOUT_S 10

uint16_t pwmSeq[4];

uint32_t buzzerActionStart;

/*Queue Size - Ensure that this is never skipped*/
#define BUZZER_QUEUE_SIZE 300

static buzzerAction_e buzzerQueue[BUZZER_QUEUE_SIZE];
static int buzzerQueueIn, buzzerQueueOut;

static void buzzerQueueInit(void)
{
    buzzerQueueIn = buzzerQueueOut = 0;
}

static int buzzerQueuePut(buzzerAction_e action)
{
    if (buzzerQueueIn == ((buzzerQueueOut - 1 + BUZZER_QUEUE_SIZE) % BUZZER_QUEUE_SIZE))
    {
        return -1; /* Queue Full*/
    }

    buzzerQueue[buzzerQueueIn] = action;

    buzzerQueueIn = (buzzerQueueIn + 1) % BUZZER_QUEUE_SIZE;

    buzzerActionStart = 0;

    return 0; // No errors
}

static bool buzzerQueueIsEmpty()
{
    return buzzerQueueIn == buzzerQueueOut;
}

static int buzzerQueueGet(buzzerAction_e *action)
{
    if (buzzerQueueIn == buzzerQueueOut)
    {
        return -1; /* Queue Empty - nothing to get*/
    }

    *action = buzzerQueue[buzzerQueueOut];

    buzzerQueueOut = (buzzerQueueOut + 1) % BUZZER_QUEUE_SIZE;

    return 0; // No errors
}

void asBuzzerInit()
{
    nrf_gpio_cfg_output(BUZZER_PIN);
    nrf_gpio_pin_clear(BUZZER_PIN);

    // Used on the MBT02 (Find it tag3)
#ifdef BUZZER_EN1_PIN
    nrf_gpio_cfg_output(BUZZER_EN1_PIN);
    nrf_gpio_pin_set(BUZZER_EN1_PIN);

    nrf_gpio_cfg_output(BUZZER_EN2_PIN);
    nrf_gpio_pin_set(BUZZER_EN2_PIN);
#endif

    // return;

    pwmSeq[0] = PWM_CH0_DUTY;
    pwmSeq[1] = PWM_CH0_DUTY;
    pwmSeq[2] = PWM_CH0_DUTY;
    pwmSeq[3] = PWM_CH0_DUTY;
    NRF_PWM0->PSEL.OUT[0] = (BUZZER_PIN << PWM_PSEL_OUT_PIN_Pos) |
                            (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);

    NRF_PWM0->ENABLE = (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
    NRF_PWM0->MODE = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);
    NRF_PWM0->PRESCALER = (PWM_PRESCALER_PRESCALER_DIV_1 << PWM_PRESCALER_PRESCALER_Pos);
    NRF_PWM0->COUNTERTOP = (PWM_PERIOD << PWM_COUNTERTOP_COUNTERTOP_Pos); // 1 msec
    // NRF_PWM0->LOOP = (PWM_LOOP_CNT_Disabled << PWM_LOOP_CNT_Pos);
    NRF_PWM0->LOOP = (1 << PWM_LOOP_CNT_Pos);
    //  NRF_PWM0->DECODER = (PWM_DECODER_LOAD_Individual << PWM_DECODER_LOAD_Pos) |
    //                      (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    NRF_PWM0->DECODER = (PWM_DECODER_LOAD_Grouped << PWM_DECODER_LOAD_Pos) | (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    NRF_PWM0->SEQ[0].PTR = ((uint32_t)(pwmSeq) << PWM_SEQ_PTR_PTR_Pos);
    NRF_PWM0->SEQ[0].CNT = ((sizeof(pwmSeq) / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos);
    NRF_PWM0->SEQ[0].REFRESH = 0;
    NRF_PWM0->SEQ[0].ENDDELAY = 0;

    buzzerQueueInit();

    buzzerActionStart = 0;

    //    NRF_PWM0->TASKS_SEQSTART[0] = 1;
}

static uint32_t buzzer_task()
{

    buzzerAction_e operation;
    // Get the operation from the queue
    if (buzzerQueueGet(&operation) == -1)
        return APP_SCHEDULER_STOP_TASK; // End of queueu = end of sequence

    // Act
    buzzerActionStart = lib_time->getTimestampS();

    switch (operation)
    {
    case BUZZER_ACTION_ON:
        LOG(LVL_DEBUG, "Buzzer ON\n");
        NRF_PWM0->TASKS_SEQSTART[0] = 1;
        return APP_SCHEDULER_SCHEDULE_ASAP;
    case BUZZER_ACTION_OFF:
        LOG(LVL_DEBUG, "Buzzer OFF\n");
        NRF_PWM0->TASKS_STOP = 1;
        return APP_SCHEDULER_SCHEDULE_ASAP;
    case BUZZER_ACTION_DELAY:
        LOG(LVL_DEBUG, "Buzzer Delay\n");
        return BUZZER_SEQUENCE_TIME_DELAY_MS;
    case BUZZER_ACTION_DELAY_SHORT:
        return BUZZER_SEQUENCE_TIME_DELAY_SHORT_MS;
    case BUZZER_ACTION_DELAY_LONG:
        return BUZZER_SEQUENCE_TIME_DELAY_LONG_MS;
    }
    // We should never arrive here
    lib_state->stopStack();
    return APP_SCHEDULER_STOP_TASK;
}

void asBuzzerBeep()
{
    bool ongoingOperation = !buzzerQueueIsEmpty();
    buzzerQueuePut(BUZZER_ACTION_ON);
    buzzerQueuePut(BUZZER_ACTION_DELAY);
    buzzerQueuePut(BUZZER_ACTION_OFF);
    if (!ongoingOperation)
    {
        App_Scheduler_addTask_execTime(buzzer_task, APP_SCHEDULER_SCHEDULE_ASAP, BUZZER_TASK_EXECUTION);
    }
}

/*This function is used to beep multiple times*/
void asBuzzerBeeps(int8_t beeps)
{
    if ((beeps * 4) > (BUZZER_QUEUE_SIZE))
    {
        LOG(LVL_ERROR, "Too much beeps\n");
        return;
    }
    bool ongoingOperation = !buzzerQueueIsEmpty();
    while (beeps-- > 0)
    {
        buzzerQueuePut(BUZZER_ACTION_ON);
        buzzerQueuePut(BUZZER_ACTION_DELAY_SHORT);
        buzzerQueuePut(BUZZER_ACTION_OFF);
        buzzerQueuePut(BUZZER_ACTION_DELAY_LONG);
    }
    if (!ongoingOperation)
    {
        App_Scheduler_addTask_execTime(buzzer_task, APP_SCHEDULER_SCHEDULE_ASAP, BUZZER_TASK_EXECUTION);
    }
}

void asBuzzerBeepShort()
{
    bool ongoingOperation = !buzzerQueueIsEmpty();
    buzzerQueuePut(BUZZER_ACTION_ON);
    buzzerQueuePut(BUZZER_ACTION_DELAY_SHORT);
    buzzerQueuePut(BUZZER_ACTION_OFF);
    if (!ongoingOperation)
    {
        App_Scheduler_addTask_execTime(buzzer_task, APP_SCHEDULER_SCHEDULE_ASAP, BUZZER_TASK_EXECUTION);
    }
}

void asBuzzerBeepDoubleShort()
{
    bool ongoingOperation = !buzzerQueueIsEmpty();
    buzzerQueuePut(BUZZER_ACTION_ON);
    buzzerQueuePut(BUZZER_ACTION_DELAY_SHORT);
    buzzerQueuePut(BUZZER_ACTION_OFF);
    buzzerQueuePut(BUZZER_ACTION_DELAY_SHORT);
    buzzerQueuePut(BUZZER_ACTION_ON);
    buzzerQueuePut(BUZZER_ACTION_DELAY_SHORT);
    buzzerQueuePut(BUZZER_ACTION_OFF);
    if (!ongoingOperation)
    {
        App_Scheduler_addTask_execTime(buzzer_task, APP_SCHEDULER_SCHEDULE_ASAP, BUZZER_TASK_EXECUTION);
    }
}

void asBuzzerBeepLong()
{
    bool ongoingOperation = !buzzerQueueIsEmpty();
    buzzerQueuePut(BUZZER_ACTION_ON);
    buzzerQueuePut(BUZZER_ACTION_DELAY_LONG);
    buzzerQueuePut(BUZZER_ACTION_OFF);
    if (!ongoingOperation)
    {
        App_Scheduler_addTask_execTime(buzzer_task, APP_SCHEDULER_SCHEDULE_ASAP, BUZZER_TASK_EXECUTION);
    }
}

#define BUZZER_RETRIEVAL_PERIOD_MS 1000

static uint32_t retrievalSecondsLeft = 0;
static bool retrievalActive = false;

static uint32_t buzzer_retrieval_task()
{
    if (retrievalSecondsLeft == 0)
    {
        LOG(LVL_DEBUG, "Buzzer retrieval done\n");
        retrievalActive = false;
        return APP_SCHEDULER_STOP_TASK;
    }

    retrievalSecondsLeft--;
    // asBuzzerBeep();
    asBuzzerBeepShort();
    return BUZZER_RETRIEVAL_PERIOD_MS;
}

/*Beep once per second for retrievalSeconds. Calling while active restarts the duration*/
void asBuzzerRetrieval(uint32_t retrievalSeconds)
{
    retrievalSecondsLeft = retrievalSeconds;
    if (!retrievalActive && retrievalSeconds > 0)
    {
        LOG(LVL_DEBUG, "Buzzer retrieval start %lu s\n", retrievalSeconds);
        retrievalActive = true;
        App_Scheduler_addTask_execTime(buzzer_retrieval_task, APP_SCHEDULER_SCHEDULE_ASAP, BUZZER_TASK_EXECUTION);
    }
}

bool asBuzzerActionOngoing()
{
    return (lib_time->getTimestampS() - buzzerActionStart) < BUZZER_ACTION_TIMEOUT_S;
}
