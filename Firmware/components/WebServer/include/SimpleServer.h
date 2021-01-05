/**
 * @file SimpleServer.h
 * @author Gustice
 * @brief Small webserver to configure the MQTT-Device and WiFi-Paramter
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

#include <esp_http_server.h>
#include "ParamRepo.h"

    void Web_StartWebserver(DeviceConfig_t * devCfg, MqttConfig_t * mqttCfg);
#ifdef __cplusplus
}
#endif