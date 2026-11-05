#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"
#include "board.h"

#include "spi.h"

#include "port_platform.h"
#include "deca_device_api.h"
// #include "deca_regs.h"
#include "dw3000_deca_regs.h"
#include "dw3000_deca_vals.h"
#include "deca_probe_interface.h"
#include "deca_interface.h"

#include "as_uwb.h"
#include "math.h"

#include "board.h"

#include "as_spi.h"
#include "gpio.h"
#include "port_platform.h"

/*----------------DEBUG-------------------------------------------*/
#define DEBUG_LOG_MODULE_NAME "AS_UWB"
#define DEBUG_LOG_MAX_LEVEL LVL_DEBUG
#include "debug_log.h"
/*----------------DEBUG-------------------------------------------*/

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

static void dw_event_cb(gpio_id_t id, gpio_in_event_e event)
{
    /*Call the ISR of the DWT System*/
    dwt_isr();
}

static void uwbInterruptSetup()
{
    /*Configure the interrupt*/
    const gpio_in_cfg_t dw_int_cfg =
        {
            .event_cb = dw_event_cb,
            .event_cfg = GPIO_IN_EVENT_RISING_EDGE,
            .in_mode_cfg = GPIO_IN_PULL_NONE};

    if (Gpio_inputSetCfg(BOARD_GPIO_ID_DW_IRQ, &dw_int_cfg) != GPIO_RES_OK)
    {

        LOG(LVL_INFO, "Cannot enable interrupt pin");
    }
    else
    {
        LOG(LVL_INFO, "Int Pin Registered");
    }
}

static void uwbInterruptDisable()
{
    /*Configure the interrupt*/
    const gpio_in_cfg_t dw_int_cfg =
        {
            .event_cb = NULL,
            .event_cfg = GPIO_IN_EVENT_NONE,
            .in_mode_cfg = GPIO_IN_PULL_NONE};

    if (Gpio_inputSetCfg(BOARD_GPIO_ID_DW_IRQ, &dw_int_cfg) != GPIO_RES_OK)
    {

        LOG(LVL_INFO, "Cannot disable interrupt pin");
    }
    else
    {
        LOG(LVL_INFO, "Int Pin Disabled");
    }
}

/*Callback to process RX good frame events*/
void rx_ok_cb(const dwt_cb_data_t *cb_data)
{
}

/*Callback to process RX timeout events*/
void rx_to_cb(const dwt_cb_data_t *cb_data)
{
}

/*Callback to process RX error events*/
void rx_err_cb(const dwt_cb_data_t *cb_data)
{
}

/*Callback to process TX confirmation events*/
void tx_conf_cb(const dwt_cb_data_t *cb_data)
{
}

static void delay_uwb(uint32_t mSeconds)
{
    app_lib_time_timestamp_hp_t start = lib_time->getTimestampHp();
    while (lib_time->getTimeDiffUs(lib_time->getTimestampHp(), start) < (mSeconds * 1000))
        ;
}

void uwbSleep()
{
    dwt_entersleep(DWT_DW_INIT);

    asSPIClose();

    uwbInterruptDisable();

    // This below was enabled but not sure if we need it anymore

    // const uint32_t pin_not_connected = 31UL;
    // /*Set the interrupt input*/
    // NRF_GPIOTE->CONFIG[INTERRUPT_UWB_IDX] = (GPIOTE_CONFIG_MODE_Disabled << GPIOTE_CONFIG_MODE_Pos) |
    //                                         (pin_not_connected << GPIOTE_CONFIG_PSEL_Pos) |
    //                                         (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos);
    // NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Disabled << INTERRUPT_UWB_POS;

    // // NRF_GPIO->PIN_CNF[DW1000_IRQ_PIN] =
    // NRF_P1->PIN_CNF[2] =
    //     (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
    //     (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos) |
    //     (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
    //     (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
    //     (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);

    // //  Disable the interrupt
    // NRF_GPIOTE->INTENCLR = INTERRUPT_UWB_INTENSET_MSK;

    // // NRF_P1->PIN_CNF[2] =
    // //     (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
    // //     (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos) |
    // //     (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
    // //     (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
    // //     (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
}

struct dwt_probe_s dw3000_probe_interf;
struct dwt_spi_s dw3000_spi_fct;

extern const struct dwt_driver_s dw3000_driver;
struct dwt_driver_s *tmp_ptr[1];

/*Initialize the DW1000 - Used once when the system starts or something has gone wrong*/
void asUWBInit()
{

    int32_t res;
    dwt_callbacks_s cbs = {NULL};

    /*Setup SPI Pins*/
    asSPIInit();

    /*Wakeup the DW1000*/
    port_wakeup_dw1000();

    /* Reset DW1000 */
    reset_DW1000();

    port_set_dw1000_slowrate();

    /*Create the probe interface*/
    tmp_ptr[0] = (struct dwt_driver_s *)&dw3000_driver;

    dw3000_spi_fct.readfromspi = readfromspi;
    dw3000_spi_fct.writetospi = writetospi;
    dw3000_spi_fct.writetospiwithcrc = writetospiwithcrc;
    dw3000_spi_fct.setslowrate = port_set_dw1000_slowrate;
    dw3000_spi_fct.setfastrate = port_set_dw1000_fastrate;

    dw3000_probe_interf.dw = NULL;
    dw3000_probe_interf.spi = &dw3000_spi_fct;
    dw3000_probe_interf.wakeup_device_with_io = port_wakeup_dw1000;
    dw3000_probe_interf.driver_list = tmp_ptr;
    dw3000_probe_interf.dw_driver_num = 1;

    LOG(LVL_INFO, "Init UWB %x", dw3000_probe_interf.driver_list[0]->devid);

    res = dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);
    LOG(LVL_INFO, "Init UWB %d", res);

    if (res != 0)
    {
        return;
    }

    /*Configure Interrupt*/
    uwbInterruptSetup();

    /* Init the DW1000 */
    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    {
        LOG(LVL_INFO, "DW1000 Error");
        return;
        LOG_FLUSH(LVL_INFO);
        delay_uwb(1000);
        // Init of DW1000 Failed
        lib_state->stopStack();
        return;
    }

    LOG(LVL_INFO, "DW OK");

    // Set SPI to 8MHz clock
    port_set_dw1000_fastrate();

    /* Configure DW1000. */
    dwt_configure(&config);

    /* Activate double buffering. */
    dwt_setdblrxbuffmode(1, DBL_BUF_MODE_MAN);

    cbs.cbTxDone = tx_conf_cb;
    cbs.cbRxOk = rx_ok_cb;
    cbs.cbRxTo = rx_to_cb;
    cbs.cbRxErr = rx_err_cb;

    dwt_setcallbacks(&cbs);

    dwt_setinterrupt(DWT_INT_TXFRS_BIT_MASK | DWT_INT_RXFCG_BIT_MASK | DWT_INT_RXFTO_BIT_MASK | DWT_INT_RXPTO_BIT_MASK | DWT_INT_RXPHE_BIT_MASK | DWT_INT_RXFCE_BIT_MASK | DWT_INT_RXFSL_BIT_MASK | DWT_INT_RXSTO_BIT_MASK,
                     0, DWT_ENABLE_INT);

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

    // dwt_write32bitreg(TX_POWER_ID, 0x25466788);
    dwt_write_reg(TX_POWER_ID, 0x25466788);
    //  LOG(LVL_INFO, "power %u", dwt_read32bitreg(TX_POWER_ID));

    dwt_configuresleep(DWT_CONFIG | DWT_PGFCAL, DWT_WAKE_CSN | DWT_SLP_EN | DWT_PRES_SLEEP);

    dwt_configciadiag(DW_CIA_DIAG_LOG_ALL);

    /*Allow the DW1000 to go to sleep*/
    uwbSleep();

    // /*Set the source address and destination address for some message buffers which do not change*/
    // lib_settings->getNodeAddress(&deviceAddress);
    // addrPtr = (uint32_t *)&uwbTXMessageBuffer[UWB_MSG_IDX_SRC_ADDRESS];
    // *addrPtr = deviceAddress;
}