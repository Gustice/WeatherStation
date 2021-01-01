/**
 * @file I2cPort.h
 * @author Gustice
 * @brief I2C-Port class
 * @version 0.1
 * @date 2020-10-10
 *
 * @copyright Copyright (c) 2020
 */

#pragma once

#include "PortBase.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

/**
 * @brief I2C-Port Class
 * @details Provides Write and Read operations via I2C-Interface
 */
class I2cPort : public PortBase
{
public:
  I2cPort();
  ~I2cPort(){};

  esp_err_t WriteData(uint16_t slaveAdd, uint8_t *data, size_t size);
  esp_err_t WriteData(uint16_t slaveAdd, uint8_t *cmd, size_t cmdSize, uint8_t *data, size_t size);

  esp_err_t ReadData(uint16_t slaveAdd, uint8_t *data, size_t size);
  esp_err_t ReadData(uint16_t slaveAdd, uint8_t *cmd, size_t cmdSize, uint8_t *data, size_t size);
};
