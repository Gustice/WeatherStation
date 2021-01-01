/**
 * @file SpiPort.h
 * @author Gustice
 * @brief SPI-Port class
 * @version 0.1
 * @date 2020-10-10
 *
 * @copyright Copyright (c) 2020
 */

#pragma once

#include "PortBase.h"
#include "driver/gpio.h"

typedef enum
{
  mode0 = 0,
  mode1 = 1,
  mode2 = 2,
  mode3 = 3,
} spi_tMmode_t;

/**
 * @brief SPI-Port Class
 * @details Provides Write and Read operations via SPI-Interface
 */
class SpiPort : public PortBase
{
public:
  SpiPort(spi_tMmode_t mode);
  ~SpiPort(){};

  esp_err_t TransmitSync(const uint8_t *txData, size_t len);
};
