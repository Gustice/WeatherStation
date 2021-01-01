/**
 * @file SpiPort.cpp
 * @author Gustice
 * @brief SPI-Port classes implementation
 * @version 0.1
 * @date 2020-10-10
 *
 * @copyright Copyright (c) 2020
 * @note: Currently only master mode is implemented
 */

#include "SpiPort.h"

#include <sys/time.h>

#include "esp8266/spi_struct.h"
#include "esp8266/gpio_struct.h"
#include "esp_system.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/spi.h"

typedef enum
{
    SPI_SEND = 0,
    SPI_RECV
} spi_master_mode_t;

#define BUFFER_MAX_SIZE 64

/* SPI transmit data, format: 8bit command (read value: 3, write value: 4) + 8bit address(value: 0x0) + 64byte data */
static void spi_master_transmit(spi_master_mode_t trans_mode, const uint8_t *data, size_t len)
{
    spi_trans_t trans;

    if (len > BUFFER_MAX_SIZE)
    {
        ESP_LOGE("SpiDrv", "ESP8266 only support transmit 64bytes one time");
        return;
    }

    memset(&trans, 0x0, sizeof(trans));
    trans.bits.val = 0; // clear all bit

    if (trans_mode == SPI_SEND)
    {
        trans.bits.mosi = len * 8; // One time transmit only support 64bytes
        trans.mosi = (uint32_t *)data;
    } // else { // @todo move to different routine
    //     trans.bits.miso = len * 8;
    //     trans.miso = (uint32_t *)data;
    // }

    trans.bits.cmd = 0;
    trans.bits.addr = 0; // transmit data will use 8bit address

    spi_trans(HSPI_HOST, &trans);
}

esp_err_t SpiPort::TransmitSync(const uint8_t *txData, size_t len)
{
    esp_err_t ret = ESP_OK;

    int fullLoops = (len) / BUFFER_MAX_SIZE;
    uint32_t loop = 0;
    for (; loop < fullLoops; loop++)
    {
        spi_master_transmit(SPI_SEND, txData + (loop * BUFFER_MAX_SIZE), BUFFER_MAX_SIZE);
    }
    int lastLoop = (len) % BUFFER_MAX_SIZE;
    if (lastLoop > 0)
    {
        spi_master_transmit(SPI_SEND, txData + (loop * BUFFER_MAX_SIZE), lastLoop);
    }
    return ret;
}

SpiPort::SpiPort(spi_tMmode_t mode) : PortBase("HAL-Spi")
{
    spi_config_t spi_config;
    // Load default interface parameters
    // CS_EN:1, MISO_EN:1, MOSI_EN:1, BYTE_TX_ORDER:1, BYTE_TX_ORDER:1, BIT_RX_ORDER:0, BIT_TX_ORDER:0, CPHA:0, CPOL:0
    spi_config.interface.val = SPI_DEFAULT_INTERFACE;
    // Load default interrupt enable
    // TRANS_DONE: true, WRITE_STATUS: false, READ_STATUS: false, WRITE_BUFFER: false, READ_BUFFER: false
    spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE;
    // Set SPI to master mode
    // ESP8266 Only support half-duplex
    spi_config.mode = SPI_MASTER_MODE;
    // Set the SPI clock frequency division factor
    spi_config.clk_div = SPI_2MHz_DIV;
    spi_config.event_cb = NULL;
    spi_init(HSPI_HOST, &spi_config);
}
