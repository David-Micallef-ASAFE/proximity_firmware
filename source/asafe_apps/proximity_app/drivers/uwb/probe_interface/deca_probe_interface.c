/*! ----------------------------------------------------------------------------
 * @file    deca_probe_interface.c
 * @brief   Platform hook-up for the DW3000 driver.
 *
 *          Only the SPI accessors and the IO wake-up function are platform
 *          dependent, so that is all this file provides.  The driver_list /
 *          dw_driver_num members of struct dwt_probe_s are gone: they pointed at
 *          dw3000_driver, whose dwt_ops_s / dwt_mcps_ops_s tables referenced every
 *          function in the driver and therefore forced the linker to keep all of
 *          it.  See dwt_attach() in deca_compat.c.
 *
 * @attention
 *
 * Copyright 2015 - 2021 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */
#include "deca_probe_interface.h"
#include "deca_interface.h"
#include "port_platform.h"

const struct dwt_spi_s dw3000_spi_fct = {
    .readfromspi = readfromspi,
    .writetospi = writetospi,
    .writetospiwithcrc = writetospiwithcrc,
    // .setslowrate = port_set_dw_ic_spi_slowrate,
    // .setfastrate = port_set_dw_ic_spi_fastrate};
    .setslowrate = port_set_dw1000_slowrate,
    .setfastrate = port_set_dw1000_fastrate};

/* Kept so that legacy code calling dwt_probe(&dw3000_probe_interf) still builds.
 * New code should call dwt_attach(&dw3000_spi_fct, port_wakeup_dw1000) instead. */
const struct dwt_probe_s dw3000_probe_interf = {
    .dw = NULL,
    .spi = (void *)&dw3000_spi_fct,
    .wakeup_device_with_io = port_wakeup_dw1000,
    .driver_list = NULL,
    .dw_driver_num = 0,
};
