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

#include "as_lis3dh.h"
#include "as_led.h"
#include "as_buzzer.h"

#include "as_gpio.h"

#include "as_dw3000.h"

/*Only enabled for testing app_setup*/
#include "app_setup.h"
// #include "provisioning.h"

#ifdef APP_BUTTON
#include "button.h"
#endif

#define DEBUG_LOG_MODULE_NAME "PROXIMITY_APP"
/** To activate logs, configure the following line with "LVL_INFO". */
#define DEBUG_LOG_MAX_LEVEL LVL_INFO

#include "debug_log.h"

/**  ID of memory area which is converted to secure storage area. */
#define APP_PERSISTENT_MEMORY_AREA_ID 0x8AE573BA

/*Global Functions*/
app_global_functions_t *global_functions;

#ifdef APP_BUTTON
static void buttonReset(uint8_t button_id, button_event_e event)
{
    lib_state->stopStack();
}
#endif

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

    LOG_INIT();
    LOG(LVL_INFO, "App_init");

    App_Setup();

    /*Used to access the global functions*/
    global_functions = (app_global_functions_t *)functions;

/*Initialize Hardware that needs to be put into sleep/low power conditions*/
/*Note that this is based on the device type*/
#ifdef CONFIG_NFCT_PINS_AS_GPIOS
    asGPIOInitNFCPinsToGPIO();
#endif

#ifdef APP_LIS3DH
    asLIS3DHInit();
#endif

#ifdef APP_LED
    asLedInit();
    asLedRunFlash();
#endif

#ifdef APP_BUZZER
    asBuzzerInit();
#endif

#ifdef APP_UWB
    asDW3000Init();
#endif

    //  At this point the device should have all the hardware in low power.
    if (Local_provisioning_is_provisioned())
    {
#ifdef APP_PRINTING
        uint32_t nodeSerialNumber;
        lib_settings->getNodeAddress(&nodeSerialNumber);
        LOG(LVL_INFO, "Node %u is provisioned", nodeSerialNumber);
#endif

        // Start processes that require provisioning
        asStatusInit();

        // Node is provisioned, enable proxy mode
        Stack_State_startStack();
    }
    else
    {
        LOG(LVL_INFO, "Provisioning Not Yet Done");
        // Provisioning init
        asProvisioningInit();
        // Start the provisioning process
        asProvisioningStart();
    }

    // To remove later on
#ifdef APP_BUTTON
    Button_register_for_event(0, BUTTON_PRESSED, buttonReset);
#endif
}
