/**
 * @file MqttDevice.h
 * @author Gustice
 * @brief MQTT-Device Functionality
 * @version 0.1
 * @date 2021-01-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    void Mqtt_ServiceStart(SemaphoreHandle_t *xSemaphore, MqttConfig_t *mqttCfg, DeviceConfig_t *defCfg);
    void Mqtt_PublishValues(uint32_t stamp, float temperature, float humidity, float pressure, int doorOpen);
    void Mqtt_PublishTemperature(float temperature);
    void Mqtt_PublishHumidity(float humidity);
    void Mqtt_PublishDoor(int doorOpen);

#ifdef __cplusplus
}
#endif