/**
 * @file I2cPort.cpp
 * @author Gustice
 * @brief I2C-Port classes implementation
 * @version 0.1
 * @date 2020-10-10
 *
 * @copyright Copyright (c) 2020
 */

#include "I2CPort.h"
#include "esp_system.h"
#include "esp_log.h"

#define ACK_CHECK_EN 0x1  /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */

I2cPort::I2cPort() : PortBase("HAL-I2c")
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = GPIO_NUM_4; //GPIO_NUM_14;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = GPIO_NUM_5; //GPIO_NUM_2;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_LOGI(cModTag, "I2C-Setup finished");
}

esp_err_t I2cPort::WriteData(uint16_t slaveAdd, uint8_t *data, size_t size)
{
    if (size == 0)
    {
        return ESP_FAIL;
    }
    if (data == nullptr)
    {
        return ESP_FAIL;
    }

    i2c_cmd_handle_t cHnd = i2c_cmd_link_create();
    i2c_master_start(cHnd);

    i2c_master_write_byte(cHnd, (slaveAdd << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write(cHnd, data, size, ACK_CHECK_EN);

    i2c_master_stop(cHnd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cHnd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cHnd);
    return ret;
    return ESP_OK;
}

esp_err_t I2cPort::ReadData(uint16_t slaveAdd, uint8_t *data, size_t size)
{
    if (size == 0)
    {
        return ESP_FAIL;
    }
    if (data == nullptr)
    {
        return ESP_FAIL;
    }

    i2c_cmd_handle_t cHnd = i2c_cmd_link_create();
    i2c_master_start(cHnd);

    i2c_master_write_byte(cHnd, (slaveAdd << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
    if (size > 1)
    {
        i2c_master_read(cHnd, data, size - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cHnd, data + size - 1, I2C_MASTER_NACK);

    i2c_master_stop(cHnd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cHnd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cHnd);
    return ret;
    return ESP_OK;
}

esp_err_t I2cPort::WriteData(uint16_t slaveAdd, uint8_t *cmd, size_t cmdSize, uint8_t *data, size_t size)
{
    if ((cmdSize == 0) || (size == 0))
    {
        return ESP_FAIL;
    }
    if ((cmd == nullptr) && (data == nullptr))
    {
        return ESP_FAIL;
    }

    i2c_cmd_handle_t cHnd = i2c_cmd_link_create();
    i2c_master_start(cHnd);

    i2c_master_write_byte(cHnd, (slaveAdd << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write(cHnd, cmd, cmdSize, ACK_CHECK_EN);
    i2c_master_write(cHnd, data, size, ACK_CHECK_EN);

    i2c_master_stop(cHnd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cHnd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cHnd);
    return ret;
    return ESP_OK;
}

esp_err_t I2cPort::ReadData(uint16_t slaveAdd, uint8_t *cmd, size_t cmdSize, uint8_t *data, size_t size)
{
    if ((cmdSize == 0) || (size == 0))
    {
        return ESP_FAIL;
    }
    if ((cmd == nullptr) && (data == nullptr))
    {
        return ESP_FAIL;
    }

    i2c_cmd_handle_t cHnd = i2c_cmd_link_create();
    i2c_master_start(cHnd);

    i2c_master_write_byte(cHnd, (slaveAdd << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write(cHnd, cmd, cmdSize, ACK_CHECK_EN);
    i2c_master_stop(cHnd);

    i2c_master_start(cHnd);
    i2c_master_write_byte(cHnd, (slaveAdd << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
    if (size > 1)
    {
        i2c_master_read(cHnd, data, size - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cHnd, data + size - 1, I2C_MASTER_NACK);

    i2c_master_stop(cHnd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cHnd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cHnd);
    return ret;
    return ESP_OK;
}
