/*! ----------------------------------------------------------------------------
 * @file    port_platform.c
 * @brief   HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2016 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include "port_platform.h"
#include "deca_device_api.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"
#include "board.h"

#include "spi.h"
#include "uwb.h"

#define SPI_SPEED_SLOWRATE 2000000
#define SPI_SPEED_FASTRATE 8000000

static volatile uint32_t signalResetDone;

uint8_t dw1000_write_buffer[1024];
uint8_t dw1000_read_buffer[1024];

spi_conf_t spi_conf;

spi_xfer_t dw10000_spi_xfer;

static void delay(uint32_t mSeconds)
{
  app_lib_time_timestamp_hp_t start = lib_time->getTimestampHp();
  while (lib_time->getTimeDiffUs(lib_time->getTimestampHp(), start) < (mSeconds * 1000))
    ;
}

/* @fn      setup_DW1000RSTnIRQ
 * @brief   setup the DW_RESET pin mode
 *          0 - output Open collector mode
 *          !0 - input mode with connected EXTI0 IRQ
 * */
void setup_DW1000RSTnIRQ(int enable)
{
}

/* @fn      port_wakeup_dw1000
 * @brief   "slow" waking up of DW1000 using DW_CS only
 * */
void port_wakeup_dw1000(void)
{
  /*Note for now we are implementing this in a simple*/
  DW1000_CS_LOW;
  delay(2);
  DW1000_CS_HIGH;
  delay(2);
}

/* @fn      port_wakeup_dw1000_fast
 * @brief   waking up of DW1000 using DW_CS and DW_RESET pins.
 *          The DW_RESET signalling that the DW1000 is in the INIT state.
 *          the total fast wakeup takes ~2.2ms and depends on crystal startup time
 * */
void port_wakeup_dw1000_fast(void)
{
}

/**
 * @brief SPI user event handler.
 * @param event
 */
// void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context)
// {
//     spi_xfer_done = true;
// }

//================================================================================================
int32_t readfromspi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t readLength, uint8_t *readBuffer)
{

  uint32 idatalength = 0;

  idatalength = headerLength + readLength;

  memset(dw1000_write_buffer, 0, idatalength);
  memset(dw1000_read_buffer, 0, idatalength);

  memcpy(&dw1000_write_buffer[0], headerBuffer, headerLength);

  dw10000_spi_xfer.write_ptr = dw1000_write_buffer;
  dw10000_spi_xfer.write_size = idatalength;

  dw10000_spi_xfer.read_ptr = dw1000_read_buffer;
  dw10000_spi_xfer.read_size = idatalength;

  DW1000_CS_LOW;

  SPI_transfer(&dw10000_spi_xfer, NULL);

  DW1000_CS_HIGH;

  // p1 = itempbuf + headerLength;

  memcpy(readBuffer, &dw1000_read_buffer[headerLength], readLength);

  return 0;
}

int32_t writetospi(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer)
{
  uint32 idatalength = 0;

  idatalength = headerLength + bodyLength;

  memset(dw1000_write_buffer, 0, idatalength);
  memset(dw1000_read_buffer, 0, idatalength);

  memcpy(&dw1000_write_buffer[0], headerBuffer, headerLength);
  memcpy(&dw1000_write_buffer[headerLength], bodyBuffer, bodyLength);

  dw10000_spi_xfer.write_ptr = dw1000_write_buffer;
  dw10000_spi_xfer.write_size = idatalength;

  dw10000_spi_xfer.read_ptr = dw1000_read_buffer;
  dw10000_spi_xfer.read_size = idatalength;

  DW1000_CS_LOW;

  SPI_transfer(&dw10000_spi_xfer, NULL);

  DW1000_CS_HIGH;

  return 0;
}

int32_t writetospiwithcrc(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer, uint8_t crc8)
{
  uint32 idatalength = 0;

  idatalength = headerLength + bodyLength + sizeof(crc8);

  memset(dw1000_write_buffer, 0, idatalength);
  memset(dw1000_read_buffer, 0, idatalength);

  memcpy(&dw1000_write_buffer[0], headerBuffer, headerLength);
  memcpy(&dw1000_write_buffer[headerLength], bodyBuffer, bodyLength);
  memcpy(&dw1000_write_buffer[headerLength + bodyLength], &crc8, 1);

  dw10000_spi_xfer.write_ptr = dw1000_write_buffer;
  dw10000_spi_xfer.write_size = idatalength;

  dw10000_spi_xfer.read_ptr = dw1000_read_buffer;
  dw10000_spi_xfer.read_size = idatalength;

  DW1000_CS_LOW;

  SPI_transfer(&dw10000_spi_xfer, NULL);

  DW1000_CS_HIGH;

  return 0;
}

//------------------------------other---------------------------

#define NRF_DRV_SPI_DEFAULT_CONFIG_2M(id)                \
  {                                                      \
      .sck_pin = CONCAT_3(SPIM, id, _SCK_PIN),           \
      .mosi_pin = CONCAT_3(SPIM, id, _MOSI_PIN),         \
      .miso_pin = CONCAT_3(SPIM, id, _MISO_PIN),         \
      .ss_pin = NRF_DRV_SPI_PIN_NOT_USED,                \
      .irq_priority = CONCAT_3(SPIM, id, _IRQ_PRIORITY), \
      .orc = 0xFF,                                       \
      .frequency = NRF_DRV_SPI_FREQ_2M,                  \
      .mode = NRF_DRV_SPI_MODE_0,                        \
      .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,      \
  }

#define NRF_DRV_SPI_DEFAULT_CONFIG_8M(id)                \
  {                                                      \
      .sck_pin = CONCAT_3(SPIM, id, _SCK_PIN),           \
      .mosi_pin = CONCAT_3(SPIM, id, _MOSI_PIN),         \
      .miso_pin = CONCAT_3(SPIM, id, _MISO_PIN),         \
      .ss_pin = NRF_DRV_SPI_PIN_NOT_USED,                \
      .irq_priority = CONCAT_3(SPIM, id, _IRQ_PRIORITY), \
      .orc = 0xFF,                                       \
      .frequency = NRF_DRV_SPI_FREQ_8M,                  \
      .mode = NRF_DRV_SPI_MODE_0,                        \
      .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,      \
  }

/* @fn      reset_DW1000
 * @brief   DW_RESET pin on DW1000 has 2 functions
 *          In general it is output, but it also can be used to reset the digital
 *          part of DW1000 by driving this pin low.
 *          Note, the DW_RESET pin should not be driven high externally.
 * */
void reset_DW1000(void)
{

  DW1000_RST_LOW;
  delay(2);
  DW1000_RST_HIGH;
  delay(2);
}

/* @fn      port_set_dw1000_slowrate
 * @brief   set 2MHz
 *          n
 * */
void port_set_dw1000_slowrate(void)
{
  /*Conformant with default config from Nordic*/
  spi_conf.clock = SPI_SPEED_SLOWRATE;
  spi_conf.mode = SPI_MODE_LOW_FIRST;
  spi_conf.bit_order = SPI_ORDER_MSB;
  // Init the SPI Hardware
  if (SPI_init(&spi_conf) == SPI_RES_ALREADY_INITIALIZED)
  {
    SPI_close();
    SPI_init(&spi_conf);
  }
  delay(2);
}

/* @fn      port_set_dw1000_fastrate
 * @brief   set 8MHz
 *
 * */
void port_set_dw1000_fastrate(void)
{
  /*Conformant with default config from Nordic*/
  spi_conf.clock = SPI_SPEED_FASTRATE;
  spi_conf.mode = SPI_MODE_LOW_FIRST;
  spi_conf.bit_order = SPI_ORDER_MSB;
  // Init the SPI Hardware
  if (SPI_init(&spi_conf) == SPI_RES_ALREADY_INITIALIZED)
  {
    SPI_close();
    SPI_init(&spi_conf);
  }
}

void deca_sleep(unsigned int time_ms)
{
  delay(time_ms);
  // nrf_delay_ms(time_ms);
}

/* Wrapper function to be used by decadriver. Declared in deca_device_api.h */
void deca_usleep(unsigned long time_us)
{
  delay(time_us / 1000);
}

// currently do nothing
decaIrqStatus_t decamutexon(void)
{
  // 	u16 j = (u16)(NVIC->ISER[0] & (1 << 5));

  // 	if(j)
  //  {
  // 		NVIC->ISER[0] &= ~(1 << 5); //disable the external interrupt line
  // 	}
  // 	return j ;

  return 0;
}

// currently do nothing
void decamutexoff(decaIrqStatus_t s)
{
  //	if(j)

  //	{
  //		NVIC->ISER[0] |= (1 << 5);;
  //	}
  ;
}

/****************************************************************************/ /**
                                                                                *
                                                                                *                          End APP port section
                                                                                *
                                                                                *******************************************************************************/

/****************************************************************************/ /**
                                                                                *
                                                                                *                              IRQ section
                                                                                *
                                                                                *******************************************************************************/

/****************************************************************************/ /**
                                                                                *
                                                                                *                              END OF IRQ section
                                                                                *
                                                                                *******************************************************************************/

/****************************************************************************/ /**
                                                                                *
                                                                                *******************************************************************************/
