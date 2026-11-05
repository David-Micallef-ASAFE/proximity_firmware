#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"
#include "board.h"

#include "as_spi.h"
#include "port_platform.h"
#include "deca_device_api.h"
#include "deca_probe_interface.h"
#include "deca_interface.h"

/*----------------DEBUG-------------------------------------------*/
#define DEBUG_LOG_MODULE_NAME "AS_UWB"
#define DEBUG_LOG_MAX_LEVEL LVL_DEBUG
#include "debug_log.h"
/*----------------DEBUG-------------------------------------------*/

// /* Default communication configuration. We use default non-STS DW mode. */
// static dwt_config_t config = {
//     5,                /* Channel number. */
//     DWT_PLEN_128,     /* Preamble length. Used in TX only. */
//     DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
//     9,                /* TX preamble code. Used in TX only. */
//     9,                /* RX preamble code. Used in RX only. */
//     0,                /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
//     DWT_BR_6M8,       /* Data rate. */
//     DWT_PHRMODE_STD,  /* PHY header mode. */
//     DWT_PHRRATE_STD,  /* PHY header rate. */
//     (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
//     DWT_STS_MODE_OFF, /* No STS mode enabled (STS Mode 0). */
//     DWT_STS_LEN_64,   /* STS length, see allowed values in Enum dwt_sts_lengths_e */
//     DWT_PDOA_M0       /* PDOA mode off */
// };

struct dwt_probe_s dw3000_probe_interf;
struct dwt_spi_s dw3000_spi_fct;

extern struct dwt_driver_s dw3000_driver;
struct dwt_driver_s drvArray[1];

void asUWBInit()
{
    // uint32_t devID;
    /*Setup SPI Pins*/
    asSPIInit();

    /*Wakeup the DW1000*/
    port_wakeup_dw1000();

    /* Reset DW1000 */
    reset_DW1000();

    port_set_dw1000_slowrate();

    /*Create the probe interface*/

    memcpy(&drvArray[0], &dw3000_driver, sizeof(dw3000_driver));

    dw3000_spi_fct.readfromspi = readfromspi;
    dw3000_spi_fct.writetospi = writetospi;
    dw3000_spi_fct.writetospiwithcrc = writetospiwithcrc;
    dw3000_spi_fct.setslowrate = port_set_dw1000_slowrate;
    dw3000_spi_fct.setfastrate = port_set_dw1000_fastrate;

    dw3000_probe_interf.dw = NULL;
    dw3000_probe_interf.spi = &dw3000_spi_fct;
    dw3000_probe_interf.wakeup_device_with_io = port_wakeup_dw1000;
    // dw3000_probe_interf.driver_list = (struct dwt_driver_s **)&drvArray;
    dw3000_probe_interf.driver_list = NULL;
    dw3000_probe_interf.dw_driver_num = 1;

    LOG(LVL_INFO, "drv num %d", dw3000_probe_interf.dw_driver_num);
    return;

    /* Probe for the correct device driver. */
    // dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);
    //  dwt_probe(&myst);

    return;

    devID = dwt_readdevid();

    // LOG(LVL_INFO, "Device ID %x", devID);
}
