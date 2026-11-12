#/* Copyright 2017 Wirepas Ltd. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 */

/*
 * \file    app.c
 * \brief   This file is a template for writing a custom application
 */

#include <stdlib.h>

#include "api.h"
#include "node_configuration.h"
#include "shared_data.h"
#include "app_scheduler.h"

#include "local_provisioning.h"
#include "stack_state.h"

#include "app_secure_storage.h"

#include "as_provisioning.h"
#include "as_status.h"
#include "as_battery.h"

#include "as_lis3dh.h"
#include "as_led.h"
#include "as_buzzer.h"

#include "as_gpio.h"

// #include "as_dw3000.h"

#include "as_button.h"

/*Only enabled for testing app_setup*/
#include "app_setup.h"
// #include "provisioning.h"

#include "as_storage.h"
#include "as_proximity.h"

#include "as_uwb.h"

#include "as_uwbTimer.h"
#include "board.h"
#include "as_spi.h"
#include "random.h"
#include "as_wirepas.h"

#include "as_manufacturing.h"

#include "as_nfc.h"
#include "as_bleBareMetal.h"

#include "as_types.h"

#ifdef APP_BUTTON
#include "button.h"
#endif

#define MANUFACTURING_NETWORK_ADDRESS 0x000002

#define DEBUG_LOG_MODULE_NAME "PROXIMITY_APP"
/** To activate logs, configure the following line with "LVL_INFO". */
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

/**  ID of memory area which is converted to secure storage area. */
#define APP_PERSISTENT_MEMORY_AREA_ID 0x8AE573BA

/*Global Functions*/
app_global_functions_t *global_functions;

/*Settings*/
extern asProximitySettings_t asProximitySettings;

#define PROVISIONING_UID (0X2A89C376)
const uint8_t provisionKeySwap[] = {
    0x77, 0x4b, 0x30, 0x78, 0xe7, 0xb7, 0x14, 0x21,
    0xd9, 0xeb, 0xdc, 0x5f, 0xe4, 0x0f, 0xfc, 0xfe,
    0x74, 0x31, 0x6b, 0xf1, 0x9e, 0x3b, 0x49, 0x9a,
    0xc6, 0x18, 0xfe, 0x9a, 0xbd, 0x69, 0x09, 0x95};

/*
In this function we will:
1) Adjust provisioning keys to Production ones
2) Switch to Inventory Mode
3) Unprovision (This triggers a reset)
*/
static void unProvisionPrepareForProduction()
{
    switch (ASAFE_DEVICE_TYPE)
    {
    case DEVICE_TYPE_ASAFE_JIG:
    case DEVICE_TYPE_ASAFE_ANCHOR_HR:
    case DEVICE_TYPE_ASAFE_ASSET_TAG_HR_WITH_RETRIEVAL:
    case DEVICE_TYPE_ASAFE_ASSET_TAG_SR_LARGE:
        // We do not unprovision the anchor as it is already provisioned
        return;
    default:
        break;
    }
    // Our first check is to see if we still have the production PK keys
    if (asProximitySettings.pskId == 0x35B3F081 && Local_provisioning_is_provisioned()) // We do not use a define here as it might get overwritten
    {
        LOG(LVL_INFO, "Applying unprovision to factory");
        // If we are here it means we need to apply changes
        // Update provisioning keys
        asProximitySettings.pskId = PROVISIONING_UID;
        memcpy(asProximitySettings.pskKey, provisionKeySwap, 32);
        // Switch to inventory
        asProximitySettings.mode = AS_MODE_INVENTORY;
        lib_settings->resetAll();
        // Reinit the settings;
        lib_settings->setNodeRole(APP_LIB_SETTINGS_ROLE_SUBNODE_LE);
        asStorageWriteProximitySettings();
        // Done
        return;
    }
    // We do nothing
    return;
}

/*
 * Put the hardware into its lowest power state while keeping the NFCT field detect interrupt alive.
 * Under Wirepas the app does not put the CPU to sleep itself, the CPU enters System ON sleep automatically
 * whenever no stack/app work is pending, so here we only make sure nothing keeps it awake.
 * System OFF is not used as NFC would then wake the device via a reset instead of the NFC interrupt.
 */
static void startLowPower()
{
    // Make sure the stack is allowed to use deep sleep (HFCLK off, RTC only)
    lib_system->disableDeepSleep(false);

#ifdef APP_UWB
    // DW3000 is put to sleep by asUWBInit but the SPIM is left enabled. It is re-opened on wakeup.
    asSPIClose();
#endif

    // LIS3DH: I2C is closed by asLIS3DHInit, the accel is powered down (motion detect only runs after asLIS3DHEnableMotionDetect)
    // LEDs: left off by asLedInit
    // NFCT: left in SENSE with FIELDDETECTED/FIELDLOST interrupts enabled by asNFCInit

    LOG(LVL_INFO, "Low power");
}

/*
 * Log why the device (re)booted from the nRF52 RESETREAS register and then clear it.
 * The register is cumulative until cleared, so it is cleared here to only report the latest reset.
 * No bit set means power-on reset or brown-out.
 */
static void logResetReason()
{
    uint32_t reason = NRF_POWER->RESETREAS;

    LOG(LVL_INFO, "Reset reason 0x%08x:%s%s%s%s%s%s%s%s",
        reason,
        (reason == 0) ? " POWER_ON/BROWNOUT" : "",
        (reason & POWER_RESETREAS_RESETPIN_Msk) ? " PIN" : "",
        (reason & POWER_RESETREAS_DOG_Msk) ? " WATCHDOG" : "",
        (reason & POWER_RESETREAS_SREQ_Msk) ? " SOFT" : "",
        (reason & POWER_RESETREAS_LOCKUP_Msk) ? " LOCKUP" : "",
        (reason & POWER_RESETREAS_OFF_Msk) ? " GPIO_WAKE" : "",
        (reason & POWER_RESETREAS_DIF_Msk) ? " DEBUG" : "",
        (reason & POWER_RESETREAS_NFC_Msk) ? " NFC_WAKE" : "");

    // Bits are cleared by writing 1 to them
    NRF_POWER->RESETREAS = reason;
}

/**  Time spent in low power before provisioning (and the stack) is started on an unprovisioned node. */
#define PROVISIONING_START_DELAY_S (60)

/*
 * Runs once PROVISIONING_START_DELAY_S after boot on an unprovisioned node.
 * Until then the stack is not started and the hardware stays in low power, giving manufacturing time to
 * switch the device back (NFC/button) before it starts provisioning.
 */
static uint32_t provisioningDelayedStartTask()
{
    LOG(LVL_INFO, "Low power wait done - starting provisioning");

    // Provisioning init
    asProvisioningInit();
    // Start the provisioning process
    asProvisioningStart();

    return APP_SCHEDULER_STOP_TASK;
}

/**
 * \brief   Initialization callback for application
 *
 * This function is called after hardware has been initialized but the
 * stack is not yet running.
 *
 */
void App_init(const app_global_functions_t *functions)
{
    (void)functions;

    uint32_t nodeSerialNumber;

    LOG_INIT();
    LOG(LVL_INFO, "App_init");
    logResetReason();

    App_Setup();

    /*This is only during development as in production we use provisioning*/
    // if (configureNodeFromBuildParameters() != APP_RES_OK)
    // {
    //     // Could not configure the node
    //     // It should not happen except if one of the config value is invalid
    //     return;
    // }

    /*Used to access the global functions*/
    global_functions = (app_global_functions_t *)functions;

    /*Read storage settings*/
    asStorageInit();

    /*Battery measurement (direct VDD or divider, per board.h)*/
    asBatteryInit();

/*Initialize Hardware that needs to be put into sleep/low power conditions*/
/*Note that this is based on the device type*/
#ifdef CONFIG_NFCT_PINS_AS_GPIOS
    asGPIOInitNFCPinsToGPIO();
#endif

    /*
       There is a possibility that a device is in Manfuacturing mode and is powered down.  This device
       would end up with manufacturing settings so we need to unprovision it in that case.  asManfucaturingInit takes care of that
        */
#ifdef APP_MANUFACTURING
    asManufacturingInit();
#endif

#ifdef APP_ASAFE_NFC
    asNFCInit();
#endif

#ifdef APP_LIS3DH
    asLIS3DHInit(); // Note that this is powered down and will not generate interrupts until asLIS3DHEnableMotionDetect() is called
#endif

#ifdef APP_LED
    asLedInit();
    asLedRunFlash();
#endif

#ifdef APP_BUZZER
    asBuzzerInit();
#endif

#ifdef APP_UWB
    // asDW3000Init();
    asUWBInit();
#ifdef APP_LOGIC_DEBUG
    asUWBConfigureDebugGPIO();
#endif
#endif

#ifdef APP_BUTTON
    asButtonInit();
#endif

#ifdef APP_BUZZER
    asBuzzerInit();
#endif

    LOG(LVL_INFO, "Battery %d mV", asBatteryGetMv());

    // We place this here to ensure that we do not have the stack started
    unProvisionPrepareForProduction();

    /*Its important that at this stage both the button and the NFC are working to enable the device to be switched between modes*/
    if (asProximitySettings.mode == AS_MODE_INVENTORY)
    {
        // The DW3000 was already put to sleep by asUWBInit. The UWB timer is deliberately NOT started here:
        // its periodic sleep check toggles the DW3000 CS line (which wakes it) and re-opens the SPIM.
        /*Go into low power*/
        startLowPower();
        return;
    }

    //  At this point the device should have all the hardware in low power.
    if (Local_provisioning_is_provisioned())
    {

        /*Start provisioning system only on devices that require it*/
        switch (ASAFE_DEVICE_TYPE)
        {
        case DEVICE_TYPE_ASAFE_JIG:
        case DEVICE_TYPE_ASAFE_ANCHOR_HR:
            LOG(LVL_INFO, "Provisioning Init");
            asProvisioningInit();
            break;
        default:
            break;
        }

        /*Initialize the Random Seed*/
        lib_settings->getNodeAddress(&nodeSerialNumber);
        Random_init(nodeSerialNumber);

#ifdef APP_PRINTING
        LOG(LVL_INFO, "Node %u is provisioned using device type %d", nodeSerialNumber, ASAFE_DEVICE_TYPE);
#endif

        // Start processes that require provisioning
        asStatusInit();

        // Start the proximity application
        asProximityInit();

        // Enable Motion for a tag only
#ifdef AS_RTLS_ROLE_TAG
#ifdef APP_LIS3DH
        asLIS3DHEnableMotionDetect();
#endif
#endif

        // Start UWB
#ifdef APP_UWB
        asTimerInit();
#endif

        asWirepasInit();

        // Node is provisioned, enable proxy mode if required
        Stack_State_startStack();
    }
    else
    {
        LOG(LVL_INFO, "Provisioning Not Yet Done");
        //   asStorageWriteProximitySettings();

        // The DW3000 was already put to sleep by asUWBInit and the UWB timer is not started (see inventory mode above).
        // On provisioning success the node restarts into the provisioned path, which starts the UWB timer.

        // Wait 1 minute in low power before activating the stack.  This gives manufacturing time to resort back
        startLowPower();
        App_Scheduler_addTask_execTime(provisioningDelayedStartTask, (PROVISIONING_START_DELAY_S * 1000), 500);
    }
}
