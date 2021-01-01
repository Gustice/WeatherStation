
/**
 * @file AdcPort.cpp
 * @author Gustice
 * @brief ADC-Port classes implementation
 * @version 0.1
 * @date 2020-10-10
 *
 * @copyright Copyright (c) 2020
 */

#include "AdcPort.h"
#include "driver/adc.h"

AdcPort::AdcPort() : PortBase("HAL-Adc") {
    adc_config_t adc_config;
    adc_config.mode = ADC_READ_TOUT_MODE;
    adc_config.clk_div = 8;
    ESP_ERROR_CHECK(adc_init(&adc_config));
}

int16_t AdcPort::ReadPort(void) {   
    uint16_t value;
    if (ESP_OK != adc_read(&value)) {
        value = 0;
    };

    return value;
}
