/*! ----------------------------------------------------------------------------
 * @file    deca_probe_interface.h
 * @brief   Platform hook-up for the DW3000 driver.
 *
 * @attention
 *
 * Copyright 2015 - 2021 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */
#ifndef DECA_PROBE_INTERFACE_H
#define DECA_PROBE_INTERFACE_H
#include "deca_device_api.h"
#include "deca_interface.h"

/* Platform SPI accessors - pass this to dwt_attach(). */
extern const struct dwt_spi_s dw3000_spi_fct;

/* Legacy: for applications still calling dwt_probe(). */
extern const struct dwt_probe_s dw3000_probe_interf;

#endif
