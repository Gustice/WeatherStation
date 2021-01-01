/**
 * @file Shtc3_Sensor.h
 * @author Gustice
 * @brief SHTC3-Sensor (Sensirion) implementation
 * @version 0.1
 * @date 2021-01-01
 * 
 * @copyright Copyright (c) 2021
 */

#pragma once

#include <stdint.h>
#include "I2cPort.h"

#define SHTC3_DEFAULT_ADDR 0x70

/**
 * Driver for the Adafruit SHTC3 Temperature and Humidity breakout board.
 */
class Shtc3_Sensor
{
public:
  Shtc3_Sensor(I2cPort *port, uint8_t address = SHTC3_DEFAULT_ADDR, bool inLowPowerMode = false);
  ~Shtc3_Sensor(void){};

  esp_err_t ReadID(uint16_t *id);
  void Reset(void);
  void LowPowerMode(bool readmode);
  esp_err_t ReadSensor(float *temperature, float *humidity);

private:
  bool _psMode = false;
  uint16_t _sensorId = 0;
  bool _idIsValid;
  I2cPort *_port = NULL;
  uint8_t _address;

  void triggerSampling(void);
  void putToSleep(void);
  void wakeUp(void);

  esp_err_t writeCommand(uint16_t command);
  esp_err_t readCommand(uint16_t command, uint8_t *buffer, uint8_t len);
  esp_err_t readValues(uint8_t *buffer, uint8_t len);
};
