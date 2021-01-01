/**
 * @file GpioPort.cpp
 * @author Gustice
 * @brief GPIO-Port classes implementation
 * @version 0.1
 * @date 2020-10-10
 *
 * @copyright Copyright (c) 2020
 * @note Interrupt based port is missing currently
 */

#include "GpioPort.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
///
/// GpioPort - Implementation
///
uint64_t GpioPort::_Reserved = 0;

GpioPort::GpioPort(gpio_num_t port) : PortBase("HAL-Gpio")
{
    _port = port;

    uint64_t p = 1ULL << port;

    if ((_Reserved & p) != 0)
    {
        ESP_LOGW(cModTag, "Gpio Port %d is already in use!", port);
    }

    _Reserved |= p;
}

GpioPort::~GpioPort()
{
    uint64_t p = 1ULL << _port;
    _Reserved &= ~p;
}

///
/// InputPort - Implementation
///
InputPort::InputPort(gpio_num_t port, GpioPort::PullResistor resistor) : GpioPort(port)
{
    memset(&_config, 0, sizeof(gpio_config_t));

    _config.mode = GPIO_MODE_INPUT;
    _config.pull_up_en = GPIO_PULLUP_DISABLE;
    _config.pull_down_en = GPIO_PULLDOWN_DISABLE;

    if (resistor == PullUp)
        _config.pull_up_en = GPIO_PULLUP_ENABLE;
    else if (resistor == PullDown)
        _config.pull_down_en = GPIO_PULLDOWN_ENABLE;

    _config.intr_type = GPIO_INTR_DISABLE;
    _config.pin_bit_mask = 1ULL << port;

    if (gpio_config(&_config) == ESP_OK)
        InitOk = true;
    else
        ESP_LOGW(cModTag, "Configuration for Gpio Port failed");
}

int InputPort::ReadPort(void) { return gpio_get_level(_port); }


///
/// OutputPort - Implementation
///
OutputPort::OutputPort(gpio_num_t port, GpioPort::OutputLogic mode) : GpioPort(port)
{
    _mode = mode;

    gpio_config_t io_conf;

    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    io_conf.intr_type = GPIO_INTR_DISABLE;

    io_conf.pin_bit_mask = 1ULL << port;
    gpio_config(&io_conf);
}

void OutputPort::WritePort(int active)
{
    if (_mode == GpioPort::Normal)
    {
        gpio_set_level(_port, active & 1);
    }
    else
    {
        active ^= 1;
        gpio_set_level(_port, active & 1);
    }
}