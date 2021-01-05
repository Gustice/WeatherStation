/**
 * @file ParamRepo.h
 * @author Gustice
 * @brief Module to read/write files from/to SPIFFS 
 * @version 0.1
 * @date 2021-01-01
 * 
 * @copyright Copyright (c) 2021
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdint.h>
#include "esp_err.h"

    extern const char *wifiCfgFile;
    extern const char *devCfgFile;
    extern const char *mqttCfgFile;

    typedef struct factoryInfo_def
    {
        char SerialNumber[16]; // Something like SN:123ABC
        char HwVersion[32]; // V 1.00.00
        char DeviceType[32];
        char SwVersion[12];
    } factoryInfo_t;

    typedef struct wifiConfig_def
    {
        uint8_t ssid[32];
        /** @note Length must be >= 8 */
        uint8_t password[64];
    } WifiConfig_t;

    typedef struct deviceConfig_def
    {
        char DeviceName[32];
        char Location[64];
    } DeviceConfig_t;

    typedef struct MqttConfig_def
    {
        char BrokerUrl[128];
        struct
        {
            char Root[128];
            char Temperature[32];
            char Humidity[32];
            char Door[32];
        } PublishTopics;
        struct {
            char Signal[128];
            char Prompt[128];
        } SubscribeTopics;
    } MqttConfig_t;

    extern const DeviceConfig_t DefaultDeviceConfig;
    extern const MqttConfig_t DefaultMqttConfig;

    void Fs_SetupSpiFFs(void);

    esp_err_t Fs_CheckIfExists(const char *file);
    esp_err_t Fs_SaveEntry(const char *file, void *stream, size_t length);
    esp_err_t Fs_ReadEntry(const char *file, void *stream, size_t length);
    esp_err_t Fs_DeleteEntry(const char *file);

    esp_err_t Fs_ReadFactoryConfiguration(factoryInfo_t * factorySet);

#ifdef __cplusplus
}
#endif