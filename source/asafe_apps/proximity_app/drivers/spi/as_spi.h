/* Copyright 2022 Redlore. All Rights Reserved.
 *
 * See file LICENSE.txt for full license details.
 *
 */

/**
 * \file as_spi.h
 *
 */

#ifndef __AS_SPI_H_
#define __AS_SPI_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api.h"

typedef enum
{
    SPI_SPEED_SLOW,
    SPI_SPEED_FAST
} spiSpeed_t;

void asSPIInit();
void asSPIClose();
void asSPIOpen(spiSpeed_t spiSpeed);

#endif