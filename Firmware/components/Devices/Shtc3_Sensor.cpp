/**
 * @file Shtc3_Sensor.cpp
 * @author Gustice
 * @brief SHTC3-Sensor (Sensirion) implementation
 * @details This Sensor Read humidity and temperature and provides data via I2C-interface.
 * @version 0.1
 * @date 2021-01-01
 * 
 * @copyright Copyright (c) 2021
 */

#include <sys/unistd.h> // for usleepCommand
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Shtc3_Sensor.h"

/// Only modes will be used where Temperature will be send first
// Normal measurement, temp first with Clock Stretch Enabled
#define NORMAL_MEAS_TFIRST_STRETCH 0x7CA2
// Low power measurement, temp first with Clock Stretch Enabled
#define LOWPOW_MEAS_TFIRST_STRETCH 0x6458

// Normal measurement, temp first with Clock Stretch disabled
#define NORMAL_MEAS_TFIRST 0x7866
// Low power measurement, temp first with Clock Stretch disabled
#define LOWPOW_MEAS_TFIRST 0x609C

#define READID 0xEFC8    /**< Read Out of ID Register */
#define SOFTRESET 0x805D /**< Soft Reset */
#define SLEEP 0xB098     /**< Enter sleep mode */
#define WAKEUP 0x3517    /**< Wakeup mode */

static const uint8_t crcTable[256] = {
    0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97,
    0xB9, 0x88, 0xDB, 0xEA, 0x7D, 0x4C, 0x1F, 0x2E,
    0x43, 0x72, 0x21, 0x10, 0x87, 0xB6, 0xE5, 0xD4,
    0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F, 0x5C, 0x6D,
    0x86, 0xB7, 0xE4, 0xD5, 0x42, 0x73, 0x20, 0x11,
    0x3F, 0x0E, 0x5D, 0x6C, 0xFB, 0xCA, 0x99, 0xA8,
    0xC5, 0xF4, 0xA7, 0x96, 0x01, 0x30, 0x63, 0x52,
    0x7C, 0x4D, 0x1E, 0x2F, 0xB8, 0x89, 0xDA, 0xEB,
    0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA,
    0x84, 0xB5, 0xE6, 0xD7, 0x40, 0x71, 0x22, 0x13,
    0x7E, 0x4F, 0x1C, 0x2D, 0xBA, 0x8B, 0xD8, 0xE9,
    0xC7, 0xF6, 0xA5, 0x94, 0x03, 0x32, 0x61, 0x50,
    0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C,
    0x02, 0x33, 0x60, 0x51, 0xC6, 0xF7, 0xA4, 0x95,
    0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D, 0x5E, 0x6F,
    0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6,
    0x7A, 0x4B, 0x18, 0x29, 0xBE, 0x8F, 0xDC, 0xED,
    0xC3, 0xF2, 0xA1, 0x90, 0x07, 0x36, 0x65, 0x54,
    0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC, 0x9F, 0xAE,
    0x80, 0xB1, 0xE2, 0xD3, 0x44, 0x75, 0x26, 0x17,
    0xFC, 0xCD, 0x9E, 0xAF, 0x38, 0x09, 0x5A, 0x6B,
    0x45, 0x74, 0x27, 0x16, 0x81, 0xB0, 0xE3, 0xD2,
    0xBF, 0x8E, 0xDD, 0xEC, 0x7B, 0x4A, 0x19, 0x28,
    0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91,
    0x47, 0x76, 0x25, 0x14, 0x83, 0xB2, 0xE1, 0xD0,
    0xFE, 0xCF, 0x9C, 0xAD, 0x3A, 0x0B, 0x58, 0x69,
    0x04, 0x35, 0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93,
    0xBD, 0x8C, 0xDF, 0xEE, 0x79, 0x48, 0x1B, 0x2A,
    0xC1, 0xF0, 0xA3, 0x92, 0x05, 0x34, 0x67, 0x56,
    0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE, 0xEF,
    0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15,
    0x3B, 0x0A, 0x59, 0x68, 0xFF, 0xCE, 0x9D, 0xAC};

static uint8_t crc8(const uint8_t *data, int len);

Shtc3_Sensor::Shtc3_Sensor(I2cPort *port, uint8_t address, bool inLowPowerMode)
{
    _port = port;
    _address = address;
    _psMode = inLowPowerMode;

    writeCommand(WAKEUP);
    vTaskDelay(10 / portTICK_PERIOD_MS);

    Reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);

    uint8_t data[3];
    _idIsValid = false;
    if (readCommand(READID, data, 3) != ESP_OK)
        return;

    if (data[2] != crc8(&data[0], 2))
        return;

    _idIsValid = true;
    _sensorId = (((uint16_t)data[0]) << 8) | (uint16_t)data[1];
}

void Shtc3_Sensor::LowPowerMode(bool readmode) { _psMode = readmode; }

esp_err_t Shtc3_Sensor::ReadID(uint16_t *id)
{
    if (_idIsValid == false)
        return ESP_FAIL;

    *id = _sensorId;
    return ESP_OK;
}

void Shtc3_Sensor::Reset(void)
{
    writeCommand(SOFTRESET);
    vTaskDelay(1 / portTICK_PERIOD_MS);
}

esp_err_t Shtc3_Sensor::ReadSensor(float *temperature, float *humidity)
{
    uint8_t readbuffer[6];
    wakeUp();

    triggerSampling();

    while (readValues(readbuffer, sizeof(readbuffer)) != ESP_OK)
    {
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    if (readbuffer[2] != crc8(&readbuffer[0], 2))
        return ESP_FAIL;
    if (readbuffer[5] != crc8(&readbuffer[3], 2))
        return ESP_FAIL;

    int32_t t = (int32_t)(((uint32_t)readbuffer[0] << 8) | readbuffer[1]);
    // T = -45 + 175*(x / 2^16)
    t = ((4375 * t) >> 14) - 4500;
    *temperature = (float)t / 100.0f;

    uint32_t rh = ((uint32_t)readbuffer[3] << 8) | readbuffer[4];
    // RH = 100 * (x / 2^16)
    rh = (625 * rh) >> 12;
    *humidity = (float)rh / 100.0f;
    putToSleep();

    return ESP_OK;
}

void Shtc3_Sensor::putToSleep(void)
{
    writeCommand(SLEEP);
}

void Shtc3_Sensor::wakeUp(void)
{
    writeCommand(WAKEUP);
    usleep(250);
}

void Shtc3_Sensor::triggerSampling(void)
{
    if (_psMode)
    {
        writeCommand(LOWPOW_MEAS_TFIRST);
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    else
    {
        writeCommand(NORMAL_MEAS_TFIRST);
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

esp_err_t Shtc3_Sensor::writeCommand(uint16_t command)
{
    uint8_t cmd[2] = {(uint8_t)(command >> 8), (uint8_t)(command & 0xFF)};
    return _port->WriteData(_address, cmd, 2);
}

esp_err_t Shtc3_Sensor::readCommand(uint16_t command, uint8_t *buffer,
                                    uint8_t len)
{
    uint8_t cmd[2] = {(uint8_t)(command >> 8), (uint8_t)(command & 0xFF)};
    return _port->ReadData(_address, cmd, 2, buffer, len);
}

esp_err_t Shtc3_Sensor::readValues(uint8_t *buffer, uint8_t len)
{
    return _port->ReadData(_address, buffer, len);
}

static uint8_t crc8(const uint8_t *data, int len)
{
    uint8_t crc(0xFF);

    for (int j = len; j; --j)
    {
        crc = crcTable[crc ^ *data++];
    }
    return crc;
}
