#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"
#include "board.h"

#include "spi.h"

void asSPIInit()
{
    // Initialize the CS PIN
    nrf_gpio_cfg_output(DW_CS_PIN);
    nrf_gpio_pin_set(DW_CS_PIN);
}

void asSPIClose()
{
    SPI_close();

    *(volatile uint32_t *)0x40003FFC = 0;
    *(volatile uint32_t *)0x40003FFC;
    *(volatile uint32_t *)0x40003FFC = 1;
}