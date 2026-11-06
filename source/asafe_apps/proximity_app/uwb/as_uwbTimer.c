/* Copyright 2022 Redlore. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"

#include "as_uwbTimer.h"
#include "board.h"

#include "as_uwb.h"
#include "as_spi.h"
#include "as_proximity.h"

#include "as_ranging.h"

#include "random.h"

#include "as_nbor.h"
#include "as_wirepas.h"
#include "local_provisioning.h"

#define MOTION_RESCAN_SECONDS 8
#define DISCOVERY_ATTEMPT_THRESHOLD 2
#define DISCOVERY_RESCAN_BACKOFF_SECONDS 5

#define POST_MOTION_FAST_RATE_SECONDS 30
#define POST_MOTION_FAST_RATE_SCANS 10
#define RANGING_ATTEMPTS 3

// This will be moved to flash
#ifndef BEACON_TRANSMIT_TICK
#define BEACON_TRANSMIT_TICK 8
#endif

/*----------------DEBUG-------------------------------------------*/
#define DEBUG_LOG_MODULE_NAME "TIMER"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO
#include "debug_log.h"
/*----------------DEBUG-------------------------------------------*/

#define DISCOVERY_CYCLE_TIME_SEC 600

typedef enum
{
    TAG_STATE_DISCOVERY,
    TAG_STATE_RANGING,
    TAG_STATE_IDLE
} tagState_e;

tagState_e tagState;

typedef enum
{
    TAG_CONDITION_STATIONARY,
    TAG_CONDITION_MOTION,
    TAG_CONDITION_OFFLINE
} tagCondition_e;

// /*States for the DW1000*/
typedef enum
{
    DW1000_STATE_SLEEP,
    DW1000_STATE_IDLE,
    DW1000_STATE_WAKEUP_ST1,
    DW1000_STATE_WAKEUP_ST2,
    DW1000_STATE_NBOR_DISCOVERY_ST1,
    DW1000_STATE_NBOR_DISCOVERY_ST2,
    DW1000_STATE_RANGING_ST1,
    DW1000_STATE_RANGING_ST2,

    DW1000_STATE_TX_DISCOVERY,
    DW1000_STATE_RANGING_TX_RSS,
    DW1000_STATE_RANGING_TX_RSE,
    DW1000_STATE_RANGING_SCHEDULE_PLANNING,
    DW1000_STATE_TX_NBOR_RST,

    DW1000_STATE_SELECT_TASK,

    DW1000_STATE_DISCOVER_NBORS,
    DW1000_STATE_WAIT_DONE,

} dw1000State_e;

// Used for Motion
tagCondition_e tagCondition;
// extern uwbTiming_t uwbTiming;    // Defined in uwb.c

#define UWB_CYCLE 5000 // mS
#define UWB_CYCLE_S (UWB_CYCLE / 1000)

#define UWB_SLOT_SIZE 50                      // mS
#define UWB_SLOT_TICKS (TIMER_100MS_TICK / 2) // 50Ms
#define UWB_SLOT_PADDING_RANGING 5            // mS
#define UWB_SLOTS_AVAILABLE (UWB_CYCLE / UWB_SLOT_SIZE)

#define LAST_SCAN_TIME_THRESHOLD 10 // Seconds

#define UWB_CYCLE_TICKS (UWB_SLOT_TICKS * UWB_SLOTS_AVAILABLE)

#define STATIONARY_SCAN_ATTEMPTS_MAX 3

bool uwbSafeSleepActivated;

bool uwbCollisionDetected;

uint32_t currentSlotStartTimestampHp; // Indicates when the running operation started

// Note that we will not 'schedule' TX_DISCOVERY as these are always every 250mS
#define SLOTS_NUM_TX_DISCOVERY 5
#define SLOTS_NUM_TOTAL 200 // 10000/50

uint8_t currentSlotIdx;
uint8_t rangingSlotIdx;

uint32_t lastRangingScan;
uint32_t lastDiscoveryScan;

bool flagStationaryNotSuccessful; // This is set to high when a stationary scan is not successful to re-scan
uint8_t stationaryScanAttempts;

extern asProximitySettings_t asProximitySettings;

static void timerAssignRangingSlot();

typedef enum
{
    UWB_CHIPSET_CONTROL_STATE_WK1,
    UWB_CHIPSET_CONTROL_STATE_WK2,
    UWB_CHIPSET_CONTROL_STATE_DIS,
    UWB_CHIPSET_CONTROL_STATE_RX,
    UWB_CHIPSET_CONTROL_STATE_TX_RSS,
    UWB_CHIPSET_CONTROL_STATE_TX_RSE,
    UWB_CHIPSET_CONTROL_STATE_TX_RST,
    UWB_CHIPSET_CONTROL_STATE_TX_DIS,
    UWB_CHIPSET_CONTROL_STATE_SLEEP,
} uwbChipsetControlTaskState_e;

typedef enum
{
    UWB_CHIPSET_REQUEST_IDLE,
    UWB_CHIPSET_REQUEST_DISCOVERY,
    UWB_CHIPSET_REQUEST_DISCOVERY_TX, // Only used on an anchor
    UWB_CHIPSET_REQUEST_RANGING,
    UWB_CHIPSET_REQUEST_NBOR_RST, // Only used on an anchor

} uwbChipsetTaskRequested_e;

uwbChipsetControlTaskState_e uwbChipsetControlTaskState;
uwbChipsetTaskRequested_e uwbChipsetTaskRequested;

bool flagReInitRequired;
bool enableRxAfterTx;

uwbNborSlot_t *requireNborRST;

static uint32_t timerNextRS()
{
    uint32_t slotsToRanging = ((SLOTS_NUM_TOTAL - currentSlotIdx) + rangingSlotIdx) % SLOTS_NUM_TOTAL;
    uint32_t timeToRanging = slotsToRanging * TIMER_SLOT_US;
    uint32_t timerOffset = timerGetSlotOffsetUs();
    if (timeToRanging < timerOffset)
    {
        timeToRanging += SLOTS_NUM_TOTAL * TIMER_SLOT_US;
    }
    timeToRanging -= timerOffset;
    LOG(LVL_INFO, "TTR %u", timeToRanging);
    return timeToRanging;
}

uint32_t uwbChipsetControlTask()
{

    while (1)
    {
        // LOG(LVL_INFO, "UWB-Task %d", uwbChipsetControlTaskState);
        switch (uwbChipsetControlTaskState)
        {
        case UWB_CHIPSET_CONTROL_STATE_WK1:
            LOG(LVL_INFO, "UWB-WK1");
            uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_WK2;
            uwbWakeUpSt1();
            uwbSafeSleepActivated = false;
            return 1;
        case UWB_CHIPSET_CONTROL_STATE_WK2:
            switch (uwbChipsetTaskRequested)
            {
            case UWB_CHIPSET_REQUEST_IDLE:
                uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_SLEEP;
                break;
            case UWB_CHIPSET_REQUEST_DISCOVERY:
                uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_DIS;
                break;
            case UWB_CHIPSET_REQUEST_DISCOVERY_TX:
                uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_TX_DIS;
                break;
            case UWB_CHIPSET_REQUEST_RANGING:
#ifdef AS_RTLS_ROLE_TAG
                uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_RX;
#endif
#ifdef AS_RTLS_ROLE_ANCHOR
                uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_TX_RSS;
#endif
                break;
            case UWB_CHIPSET_REQUEST_NBOR_RST:
                uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_TX_RST;
                break;
            }
            uwbWakeUpSt2();
            LOG(LVL_INFO, "UWB-WK2");
            return 3;
        case UWB_CHIPSET_CONTROL_STATE_DIS:
            uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_SLEEP;
            uwbEnableRX();
            LOG(LVL_INFO, "UWB-DIS");
            return 250;
            break;
        case UWB_CHIPSET_CONTROL_STATE_RX:
            uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_SLEEP;
#ifdef APP_LOGIC_DEBUG
            nrf_gpio_pin_set(SIGNAL_B_PIN);
#endif
            uwbEnableRX();
            lastRangingScan = lib_time->getTimestampS();
            LOG(LVL_INFO, "UWB-RX");
            return 28;
        case UWB_CHIPSET_CONTROL_STATE_TX_RSS:
            uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_TX_RSE;
            enableRxAfterTx = true;
#ifdef APP_LOGIC_DEBUG
            nrf_gpio_pin_set(SIGNAL_B_PIN);
#endif
            uwbTransmitMsg(UWB_FUNCTION_CODE_RSS, rangingSlotSequence(), UWB_ADDRESS_BROADCAST, timerNextRS());
            LOG(LVL_INFO, "UWB-TX RSS %d", rangingSlotSequence());
            return 25;
            break;
        case UWB_CHIPSET_CONTROL_STATE_TX_RSE:
            uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_SLEEP;
            enableRxAfterTx = false;
            uwbDisableRX();
#ifdef APP_LOGIC_DEBUG
            nrf_gpio_pin_set(SIGNAL_B_PIN);
#endif
            uwbTransmitMsg(UWB_FUNCTION_CODE_RSE, rangingSlotSequence(), UWB_ADDRESS_BROADCAST, timerNextRS());
            App_Scheduler_addTask_execTime(rangingSlotReady, 2, 500); // Delay this by 2mS
            LOG(LVL_INFO, "UWB-TX RSE");
            return 2;
            break;
        case UWB_CHIPSET_CONTROL_STATE_TX_RST:
            uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_SLEEP;
            enableRxAfterTx = false;
#ifdef APP_LOGIC_DEBUG
            nrf_gpio_pin_set(SIGNAL_A_PIN);
#endif
            uwbTransmitMsg(UWB_FUNCTION_CODE_RST, 0, requireNborRST->address, timerNextRS()); // Still to add dst address
            LOG(LVL_INFO, "UWB-TX RST");
            return 2;
            break;
        case UWB_CHIPSET_CONTROL_STATE_TX_DIS:
            uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_SLEEP;
            enableRxAfterTx = false;
            uwbTransmitMsg(UWB_FUNCTION_CODE_NBOR_DISCOVERY, 0, UWB_ADDRESS_BROADCAST, timerNextRS()); // Still to add dst address
            LOG(LVL_INFO, "UWB-TX DIS");
            return 2;
            break;
        case UWB_CHIPSET_CONTROL_STATE_SLEEP:
            LOG(LVL_INFO, "UWB-SLEEP");
#ifdef APP_LOGIC_DEBUG
            nrf_gpio_pin_clear(SIGNAL_A_PIN);
            nrf_gpio_pin_clear(SIGNAL_B_PIN);
#endif
            // Check if we have comms with DW1000
            flagReInitRequired = !uwbDisableRX();
            asUWBSleep();
            // if (uwbChipsetTaskRequested == UWB_CHIPSET_REQUEST_RANGING)
            // {
            //     flagStationaryNotSuccessful = !rangingSlotReady();
            //     if (flagStationaryNotSuccessful)
            //     {
            //         stationaryScanAttempts++;
            //     }
            //     if (!rangingCommsReceived())
            //     {
            //         // Check if we have received any data, if not re-init to be safe.
            //         flagReInitRequired = true;
            //     }
            // }
            if (uwbChipsetTaskRequested == UWB_CHIPSET_REQUEST_DISCOVERY || uwbChipsetTaskRequested == UWB_CHIPSET_REQUEST_RANGING)
            {
                LOG(LVL_INFO, "Printing Slots");
                nborManagerPrintSlots();
            }
#ifdef AS_RTLS_ROLE_TAG
            nborManagerProcessForBestAnchor();
            if (uwbChipsetTaskRequested == UWB_CHIPSET_REQUEST_RANGING)
            {
                rangingSlotReady();
                // App_Scheduler_addTask_execTime(rangingSlotReady, APP_SCHEDULER_SCHEDULE_ASAP, 500); // Delay this by 2mS
            }
#endif
            uwbChipsetTaskRequested = UWB_CHIPSET_REQUEST_IDLE;
            LOG(LVL_INFO, "UWB-IDLE");
            return APP_SCHEDULER_STOP_TASK;
        default:
            LOG(LVL_INFO, "UWB-Unknown State");
            return APP_SCHEDULER_STOP_TASK;
            break;
        }
    }
}

#define WIREPAS_DISCONNECTED_TIME_SECONDS 300

bool requestMotionScan;
bool requestStationaryScan;

/*
 * Period of the UWB sleep guard while UWB is idle.
 * A sleeping DW3000 cannot be queried without waking it (DWT_WAKE_CSN: any CS low edge is a wake request),
 * so instead of polling its state, it is periodically re-initialised, which always ends with it in sleep.
 */
#define UWB_SLEEP_GUARD_PERIOD_S 600

uint32_t uwbReInitTask()
{
    LOG(LVL_INFO, "UWB-REINIT");
    // asUWBInit resets the DW3000 and always ends by putting it to sleep
    asUWBInit();
    // The SPIM is left enabled by asUWBInit, close it. It is re-opened by the next wakeup.
    asSPIClose();
    return APP_SCHEDULER_STOP_TASK;
}

// uint32_t devices[8] = {111151473, 59005298, 1590757103, 352154702, 1379507639, 186557761, 1921032838, 1330726730};

#ifdef AS_RTLS_ROLE_ANCHOR
static uint32_t getDeviceOffset(uwbNborSlot_t *nborSlot)
{

    if (asProximitySettings.rstSlotOffset == 0)
    {
        // This is the default and in this case we will simply use a random offset
        return nborSlot->slotOffset + (Random_get32() % 20);
    }
    // If we are here it means we have been assigned an offset
    return nborSlot->slotOffset + (asProximitySettings.rstSlotOffset % 20);
}
#endif

#define TIMER_TICK_TASK_EXEC_TIME_US 500

/*
 * Slot processing, run as a scheduler task (task context) from the RTC0 interrupt.
 * Stack services, SPI and logging are not safe from the fast interrupt, so everything
 * apart from the slot timing itself is done here.
 */
static uint32_t timerTickTask()
{

    uint32_t currentTimestamp = lib_time->getTimestampS();
    static uint32_t lastConnectionToSinkTimestamp = 0;
    static uint32_t lastSleepCheckTimestamp = 0;

#ifdef AS_RTLS_ROLE_TAG
    uwbNborSlot_t *requireNborRST;
    bool isMotionScan = false;
#endif

    // If we are connected to the sink we establish the last time we had a connection to the sink. This is used to determine if we are offline or not.
    if (asWirepasSinkRouteEstablished())
    {
        lastConnectionToSinkTimestamp = currentTimestamp;
    }

    if (currentSlotIdx % 20 == 0)
    {
        LOG(LVL_INFO, "Wirepas %s", asWirepasSinkRouteEstablished() ? "Connected" : "Disconnected");
    }

    // WP connection and UWB Operation
    if ((!asWirepasSinkRouteEstablished() && (lib_time->getTimestampS() - lastConnectionToSinkTimestamp) > WIREPAS_DISCONNECTED_TIME_SECONDS) || !asProximitySettings.uwbOperationEnabled || asProximitySettings.mode == AS_MODE_INVENTORY)
    // if ((!asWirepasSinkRouteEstablished() && (lib_time->getTimestampS() - lastConnectionToSinkTimestamp) > WIREPAS_DISCONNECTED_TIME_SECONDS))
    {
        // LOG(LVL_INFO, "WP Disconnected");
        //  Ensure device is in sleep.
        if (uwbChipsetTaskRequested == UWB_CHIPSET_REQUEST_IDLE)
        {
            if ((currentTimestamp - lastSleepCheckTimestamp) > UWB_SLEEP_GUARD_PERIOD_S)
            {
                lastSleepCheckTimestamp = currentTimestamp;
                LOG(LVL_INFO, "UWB Sleep Guard");
                App_Scheduler_addTask_execTime(uwbReInitTask, APP_SCHEDULER_SCHEDULE_ASAP, 500);
            }
        }
    }
    else
    {
#ifdef AS_RTLS_ROLE_TAG

        if (requestStationaryScan || requestMotionScan)
        {
            // Discovery
            if ((currentTimestamp - lastDiscoveryScan) >= 10)
            {
                LOG(LVL_INFO, "DISCOVERY");
                lastDiscoveryScan = lib_time->getTimestampS();
                uwbChipsetTaskRequested = UWB_CHIPSET_REQUEST_DISCOVERY;
                uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_WK1;
                App_Scheduler_addTask_execTime(uwbChipsetControlTask, APP_SCHEDULER_SCHEDULE_ASAP, 500);
            }
            // Ranging
            if (uwbChipsetTaskRequested == UWB_CHIPSET_REQUEST_IDLE)
            {
                // Check if we have a valid nbor (SA) to work with
                if ((requireNborRST = nborManagerRSAvailable(currentSlotIdx)) != NULL)
                {
                    // Motion
                    if (requestMotionScan)
                    {
                        requestMotionScan = false;
                        isMotionScan = true;
                        LOG(LVL_INFO, "R Motion %u %d", requireNborRST->address, requireNborRST->nbors);
                        uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_WK1;
                        uwbChipsetTaskRequested = UWB_CHIPSET_REQUEST_RANGING;
                        rangingStart(isMotionScan);
                        App_Scheduler_addTask_execTime(uwbChipsetControlTask, requireNborRST->slotOffset - 2, 500);
                    }
                    else if (requestStationaryScan && requireNborRST->isBestNbor)
                    {
                        requestStationaryScan = false;
                        isMotionScan = false;
                        LOG(LVL_INFO, "R Stationary %u %d", requireNborRST->address, requireNborRST->nbors);
                        uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_WK1;
                        uwbChipsetTaskRequested = UWB_CHIPSET_REQUEST_RANGING;
                        rangingStart(isMotionScan);
                        App_Scheduler_addTask_execTime(uwbChipsetControlTask, requireNborRST->slotOffset - 2, 500);
                    }
                }
                else if (!nborManagerNborsAvailable())
                {
                    // Since we are not near any HR Anchors, we have to abondon
                    LOG(LVL_INFO, "No Nbors Available!");
                    requestStationaryScan = false;
                    requestMotionScan = false;
                }
            }
        }
        else if (uwbChipsetTaskRequested == UWB_CHIPSET_REQUEST_IDLE)
        {
            if ((currentTimestamp - lastSleepCheckTimestamp) > UWB_SLEEP_GUARD_PERIOD_S)
            {
                lastSleepCheckTimestamp = currentTimestamp;
                LOG(LVL_INFO, "UWB Sleep Guard");
                App_Scheduler_addTask_execTime(uwbReInitTask, APP_SCHEDULER_SCHEDULE_ASAP, 500);
            }
        }

#endif
// Anchor
#ifdef AS_RTLS_ROLE_ANCHOR
        static uint32_t lastDiscoveryTimestamp;
        static bool startupDiscoveryDone;
        if (uwbCollisionDetected)
        {
            LOG(LVL_INFO, "Collision Detected");
            uwbCollisionDetected = false;
            timerAssignRangingSlot();
        }
        // We can only requires a new operation if we are idle
        if (uwbChipsetTaskRequested == UWB_CHIPSET_REQUEST_IDLE)
        {
            if (currentSlotIdx == rangingSlotIdx)
            {
                LOG(LVL_INFO, "Run: Ranging");
                uwbChipsetTaskRequested = UWB_CHIPSET_REQUEST_RANGING;
                uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_WK1;
                App_Scheduler_addTask_execTime(uwbChipsetControlTask, APP_SCHEDULER_SCHEDULE_ASAP, 500);
            }
            else if ((requireNborRST = nborManagerRequireNborRST(currentSlotIdx)) != NULL)
            {
                LOG(LVL_INFO, "Run: NBOR RST");
                uwbChipsetTaskRequested = UWB_CHIPSET_REQUEST_NBOR_RST;
                uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_WK1;
                App_Scheduler_addTask_execTime(uwbChipsetControlTask, 3 + getDeviceOffset(requireNborRST), 500);
            }
            else if ((currentSlotIdx % 5) == 0)
            {
                uwbChipsetTaskRequested = UWB_CHIPSET_REQUEST_DISCOVERY_TX;
                uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_WK1;
                App_Scheduler_addTask_execTime(uwbChipsetControlTask, APP_SCHEDULER_SCHEDULE_ASAP, 500);
                LOG(LVL_INFO, "Run: TX Discovery");
            }
            else if ((currentTimestamp - lastDiscoveryTimestamp) > DISCOVERY_CYCLE_TIME_SEC)
            {
                lastDiscoveryTimestamp = currentTimestamp;
                uwbChipsetTaskRequested = UWB_CHIPSET_REQUEST_DISCOVERY;
                uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_WK1;
                App_Scheduler_addTask_execTime(uwbChipsetControlTask, APP_SCHEDULER_SCHEDULE_ASAP, 500);
            }
            else if (!startupDiscoveryDone && currentTimestamp > 30) // We wait 30 seconds to allow the system to settle before doing a discovery scan
            {
                LOG(LVL_INFO, "Run: Startup Discovery");
                startupDiscoveryDone = true;
                lastDiscoveryTimestamp = currentTimestamp;
                uwbChipsetTaskRequested = UWB_CHIPSET_REQUEST_DISCOVERY;
                uwbChipsetControlTaskState = UWB_CHIPSET_CONTROL_STATE_WK1;
                App_Scheduler_addTask_execTime(uwbChipsetControlTask, APP_SCHEDULER_SCHEDULE_ASAP, 500);
            }
        }
        else if (currentSlotIdx == rangingSlotIdx)
        {
            // If we are here it means we have a collision of some event with the ranging slot.
            if (uwbChipsetTaskRequested == UWB_CHIPSET_REQUEST_NBOR_RST)
            {
                // It means we have a collision of the Ranging slot with a NBOR_RST. We will skip the NBOR_RST and re-assign ranging slot
                LOG(LVL_INFO, "Ranging Slot Collision with NBOR_RST. Re-assigning Ranging Slot");
                timerAssignRangingSlot();
            }
            if (uwbChipsetTaskRequested == UWB_CHIPSET_REQUEST_DISCOVERY_TX)
            {
                // Every slot is 50mS and the discovery slot occurs at 5, 10, 15 and ranging has priority.  We should not get here but just in case.
                LOG(LVL_INFO, "Ranging Slot Collision with TX Dis. Re-assigning Ranging Slot");
                timerAssignRangingSlot();
            }
        }
#endif
    }
    return APP_SCHEDULER_STOP_TASK;
}

/*
 * Fast interrupt: only keep the slot timing here and defer the rest to timerTickTask.
 */
static void RTC0_interrupt_handler(void)
{
    if (NRF_RTC0->EVENTS_COMPARE[0] != 0)
    {
        NRF_RTC0->EVENTS_COMPARE[0] = 0;

        NRF_RTC0->CC[0] += TIMER_SLOT_TICK;
        currentSlotIdx++;
        currentSlotIdx %= SLOTS_NUM_TOTAL; // Wrap around

        currentSlotStartTimestampHp = lib_time->getTimestampHp();

        App_Scheduler_addTask_execTime(timerTickTask, APP_SCHEDULER_SCHEDULE_ASAP, TIMER_TICK_TASK_EXEC_TIME_US);
    }
}

/*This function is called once to asign a ranging slot within the available options*/
static void timerAssignRangingSlot()
{
    uint32_t deviceAddress;
    lib_settings->getNodeAddress(&deviceAddress);

    // Assign the ranging slot
    rangingSlotIdx = Random_get16() % SLOTS_NUM_TOTAL;

    LOG(LVL_INFO, "Assigned Slot %d", rangingSlotIdx);
}

void asTimerInit()
{
    // Setup the Timer Interrupt
    NRF_RTC0->TASKS_STOP = 1;
    NRF_RTC0->TASKS_CLEAR = 1;
    NRF_RTC0->CC[0] = 0;
    NRF_RTC0->EVENTS_COMPARE[0] = 0;

    lib_system->clearPendingFastAppIrq(RTC0_IRQn);
    if (lib_system->enableAppIrq(true, RTC0_IRQn, APP_LIB_SYSTEM_IRQ_PRIO_LO, RTC0_interrupt_handler) != APP_RES_OK)
    {
        // Error
        LOG(LVL_INFO, "Error enabling UWB Interrupt");
        return;
    }

    NRF_RTC0->PRESCALER = TIMER_PRESCALER;
    // NRF_RTC0->INTENSET = RTC_INTENSET_TICK_Msk;
    NRF_RTC0->TASKS_CLEAR = 1;
    NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos; // taken from Nordic dev zone
    NRF_RTC0->EVTENSET = RTC_EVTENSET_COMPARE0_Enabled << RTC_EVTENSET_COMPARE0_Pos;
    NRF_RTC0->CC[0] = TIMER_1S_TICK;
    NRF_RTC0->TASKS_START = 1;

    rangingInit();

    nborManagerInit();

    timerAssignRangingSlot();

    currentSlotIdx = 0;
    lastRangingScan = 0;
    lastDiscoveryScan = 0;
    uwbChipsetTaskRequested = UWB_CHIPSET_REQUEST_IDLE;

    uwbCollisionDetected = false;
}

void timerStop()
{
    NRF_RTC0->TASKS_CLEAR = 1;
    NRF_RTC0->TASKS_STOP = 1;
}

uint32_t timerGetSlotOffsetUs()
{
    uint32_t currentTimestampHP = lib_time->getTimestampHp();
    return lib_time->getTimeDiffUs(currentTimestampHP, currentSlotStartTimestampHp);
}

int16_t timerGetSlotForNbor(uint16_t nextSlotIn, uint8_t nborSlotIdx)
{
    uint16_t idealSlotIdx = (currentSlotIdx + nextSlotIn) % SLOTS_NUM_TOTAL;
    return idealSlotIdx;
}

void asUWBTimerRequestMotionScan()
{
    requestMotionScan = true;
}

void asUWBTimerRequestStationaryScan()
{
    requestStationaryScan = true;
}

void asUWBTimerInidicateCollisionDetected()
{
    uwbCollisionDetected = true;
}
