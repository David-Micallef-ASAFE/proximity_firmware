#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"
#include "board.h"

#include "spi.h"
#include "as_spi.h"

#include "math.h"

#include "DW3000_constants.h"
#include "DW3000_types.h"

/*----------------DEBUG-------------------------------------------*/
#define DEBUG_LOG_MODULE_NAME "AS_DW3000"
#define DEBUG_LOG_MAX_LEVEL LVL_DEBUG
#include "debug_log.h"
/*----------------DEBUG-------------------------------------------*/

#define SPI_SPEED_SLOWRATE 2000000
#define SPI_SPEED_FASTRATE 8000000

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

static void asDW3000HardReset()
{
    nrf_gpio_pin_clear(DW_RST_PIN);
    delay(2);
    nrf_gpio_pin_set(DW_RST_PIN);
    delay(2);
}

/*
 Internal helper function to send and receive Bytes through the SPI Interface
 @param b The bytes to be sent as an array
 @param lenB The length of the array b
 @param recLen The length of bytes that should be received back after sending the data b
 @return Returns 0 or the received bytes from the chip
 */
uint32_t sendBytes(uint8_t *buf, int lenB, int recLen)
{
    uint32_t idatalength = 0;
    uint32_t *recvData;

    idatalength = lenB + recLen;

    memset(dw1000_write_buffer, 0, idatalength);
    memset(dw1000_read_buffer, 0, idatalength);

    memcpy(&dw1000_write_buffer[0], buf, lenB);

    dw10000_spi_xfer.write_ptr = dw1000_write_buffer;
    dw10000_spi_xfer.write_size = idatalength;

    dw10000_spi_xfer.read_ptr = dw1000_read_buffer;
    dw10000_spi_xfer.read_size = idatalength;

    nrf_gpio_pin_clear(DW_CS_PIN);

    SPI_transfer(&dw10000_spi_xfer, NULL);

    nrf_gpio_pin_set(DW_CS_PIN);

    if (recLen > 0)
    {
        recvData = (uint32_t *)&dw1000_read_buffer[lenB];
        return *recvData;
    }

    return 0;
}

/*
 Helper function to count the bits of a number
 return length of a number in bits
*/
unsigned int countBits(unsigned int number)
{
    return (int)log2(number) + 1;
}

uint8_t headerArr[2];
uint8_t payload[250];

/*
 Helper function to read or write to a specific chip register address
 @param base The base register address
 @param sub  The sub register address
 @param data The data that should be written (if anything should be written, else left at 0)
 @param dataLen The length of the data that should be sent in bits. Can be left at zero to automatically determine the data length,
        but can be very useful if e.g. 32 bits of 0x00 should be written
 @param readWriteBit Defines if a read or write operation will be performed. 0 if data should be read back, 1 if only a write should be performed.
 @return Returns 0 or the result of the read operation
*/
uint32_t readOrWriteFullAddress(uint32_t base, uint32_t sub, uint32_t data, uint32_t dataLen, uint32_t readWriteBit)
{
    uint32_t header = 0x00;

    if (readWriteBit)
        header = header | 0x80;

    header = header | ((base & 0x1F) << 1);

    if (sub > 0)
    {
        header = header | 0x40;
        header = header << 8;
        header = header | ((sub & 0x7F) << 2);
    }

    uint32_t header_size = header > 0xFF ? 2 : 1;
    uint32_t res = 0;

    if (!readWriteBit)
    {
        if (header_size == 1)
        {
            headerArr[0] = header;
        }
        else
        {
            headerArr[0] = (header & 0xFF00) >> 8;
            headerArr[1] = header & 0xFF;
        }

        res = (uint32_t)sendBytes(&headerArr[0], header_size, 4);
        return res;
    }
    else
    {
        uint32_t payload_bytes = 0;
        if (dataLen == 0)
        {
            if (data > 0)
            {
                uint32_t payload_bits = countBits(data);
                payload_bytes = (payload_bits - (payload_bits % 8)) / 8; // calc the used bytes for transaction
                if ((payload_bits % 8) > 0)
                {
                    payload_bytes++;
                }
            }
            else
            {
                payload_bytes = 1;
            }
        }
        else
        {
            payload_bytes = dataLen;
        }

        if (header_size == 1)
        {
            payload[0] = header;
        }
        else
        {
            payload[0] = (header & 0xFF00) >> 8;
            payload[1] = header & 0xFF;
        }

        for (uint32_t i = 0; i < payload_bytes; i++)
        {
            payload[header_size + i] = (data >> i * 8) & 0xFF;
        }

        res = (uint32_t)sendBytes(payload, 2 + payload_bytes, 0); // "2 +" because the first 2 bytes are the header part
        return res;
    }
}

/*
 Reads from a specific chip register address
 @param base The chips base register address
 @param sub The chips sub register address
 @return The result of the read operation
*/
uint32_t read(uint32_t base, uint32_t sub)
{
    uint32_t tmp;
    tmp = readOrWriteFullAddress(base, sub, 0, 0, 0);
    return tmp;
}

/*
 Writes to a specific chip register address and automatically determines the dataLen parameter
 @param base The chips base register address
 @param sub The chips sub register address
 @param data The data that should be written
 @return The result of the write operation (typically 0)
*/
uint32_t write(uint32_t base, uint32_t sub, uint32_t data)
{
    return readOrWriteFullAddress(base, sub, data, 0, 1);
}

static int checkForDevID()
{
    uint32_t res = read(GEN_CFG_AES_LOW_REG, NO_OFFSET);
    LOG(LVL_INFO, "DEVID %x", res);
    return res;
}

static void port_set_dw1000_slowrate(void)
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

static void dw3000Sleep()
{
    /*This is a very basic configuration to put the DW3000 into sleep*/

    // Configure Sleep
    write(AON_REG, 0x14, 0x03);

    write(AON_REG, 0x04, 0);
    write(AON_REG, 0x04, AON_CTRL_ARRAY_SAVE_BIT_MASK);
}

void asDW3000Init()
{
    /*Setup the RST and CS PINs*/
    nrf_gpio_cfg_output(DW_RST_PIN);
    nrf_gpio_pin_set(DW_RST_PIN);
    nrf_gpio_cfg_output(DW_CS_PIN);
    nrf_gpio_pin_set(DW_CS_PIN);

    /*Reset the DW3000*/
    asDW3000HardReset();

    /*Initialize the SPI*/
    asSPIInit();

    /*Open SPI*/
    port_set_dw1000_slowrate();

    /*Check Device ID*/
    checkForDevID();

    /*Put the device to sleep*/
    dw3000Sleep();

    asSPIClose();
    asSPIInit();
}