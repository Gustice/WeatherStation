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

    typedef struct wifiConfig_def
    {
        uint8_t ssid[32];
        /** @note Length >= 8 */
        uint8_t password[64];
    } WifiConfig_t;

    typedef struct deviceConfig_def
    {
        char DeviceName[64];
        char Location[128];
    } DeviceConfig_t;

    typedef struct versionInfo_def
    {
        char HwVersion[12]; // V 1.00.00
        char SwVersion[12];
        char SerialNumber[12]; // Something like SN:123ABC
    } VersionInfo_t;

    typedef struct MqttConfig_def
    {
        char BrokerUrl[256];
        struct
        {
            char Root[256];
            char Temperature[64];
            char Humidity[64];
            char Door[64];
        } Topics;
    } MqttConfig_t;

    void setupSpiFFs(void);
    void unmountSpiFFs(void);

    esp_err_t Fs_CheckIfExists(const char *file);
    esp_err_t Fs_SaveEntry(const char *file, void *stream, size_t length);
    esp_err_t Fs_ReadEntry(const char *file, void *stream, size_t length);
    esp_err_t Fs_DeleteEntry(const char *file);

#ifdef __cplusplus
}
#endif