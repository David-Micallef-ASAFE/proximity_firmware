#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"
#include "board.h"

#include "spi.h"
#include "as_spi.h"

#define SPI_SPEED_SLOWRATE 2000000
#define SPI_SPEED_FASTRATE 8000000

spi_conf_t spi_conf;

void asSPIInit()
{
    // Initialize the CS PIN
    nrf_gpio_cfg_output(DW_CS_PIN);
    nrf_gpio_pin_set(DW_CS_PIN);
}

void asSPIOpen(spiSpeed_t spiSpeed)
{
    spi_conf.clock = spiSpeed == SPI_SPEED_FAST ? SPI_SPEED_FASTRATE : SPI_SPEED_SLOWRATE;
    spi_conf.mode = SPI_MODE_LOW_FIRST;
    spi_conf.bit_order = SPI_ORDER_MSB;
    // Init the SPI Hardware
    if (SPI_init(&spi_conf) == SPI_RES_ALREADY_INITIALIZED)
    {
        SPI_close();
        SPI_init(&spi_conf);
    }
}

void asSPIClose()
{
    SPI_close();

    *(volatile uint32_t *)0x40003FFC = 0;
    *(volatile uint32_t *)0x40003FFC;
    *(volatile uint32_t *)0x40003FFC = 1;
}