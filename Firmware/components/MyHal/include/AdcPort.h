/**
 * @file AdcPort.h
 * @author Gustice
 * @brief ADC-Port class
 * @version 0.1
 * @date 2020-10-10
 *
 * @copyright Copyright (c) 2020
 */

#pragma once

#include "PortBase.h"
#include "driver/adc.h"
#include "driver/gpio.h"

/**
 * @brief Analogue-to-Digital-Converter-Port Class
 * @details Provides Read on analog ports
 */
class AdcPort : public PortBase
{
public:
  AdcPort();
  ~AdcPort(){};

  int16_t ReadPort(void);
};
