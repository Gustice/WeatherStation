/**
 * @file MqttDevice.c
 * @author Gustice
 * @brief MQTT-Device Functionality
 * @details Derived from Espressif-Example "\examples\protocols\mqtt\tcp"
 * @version 0.1
 * @date 2021-01-01
 * 
 * @copyright Copyright (c) 2021
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "WifiConnect.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "cJSON.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "MqttDevice.h"

static const char *TAG = "mqttDevice";

static bool FreeToPublish = false;
static SemaphoreHandle_t *_xSemaphore = NULL;
static MqttConfig_t MqttConfig;
static DeviceConfig_t *pDeviceConfig;
static esp_mqtt_client_handle_t _client = NULL;

void publishBootUpMsg(void)
{
    cJSON *root;
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", pDeviceConfig->DeviceName);
    cJSON_AddStringToObject(root, "location", pDeviceConfig->Location);
    char *rendered = cJSON_PrintUnformatted(root); // to save pretty whitespaces cJSON_Print
    cJSON_Delete(root);

    char topicBuf[320];
    sprintf(topicBuf, "%s%s%s", "bootup/", MqttConfig.PublishTopics.Root, "Device");

    if (esp_mqtt_client_publish(_client, topicBuf, rendered, 0, 1, 0) == -1)
        ESP_LOGW(TAG, "Sending of 'AllData' failed");
    free(rendered);
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msgId;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        FreeToPublish = true;
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msgId = esp_mqtt_client_subscribe(client, MqttConfig.SubscribeTopics.Signal, 0);
        if (msgId != -1)
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msgId);

        msgId = esp_mqtt_client_subscribe(client, MqttConfig.SubscribeTopics.Prompt, 0);
        if (msgId != -1)
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msgId);


        ESP_LOGI(TAG, "Publishing Temperature on %s%s", MqttConfig.PublishTopics.Root, MqttConfig.PublishTopics.Temperature);
        ESP_LOGI(TAG, "Publishing Humidity on %s%s", MqttConfig.PublishTopics.Root, MqttConfig.PublishTopics.Temperature);
        ESP_LOGI(TAG, "Publishing Door on %s%s", MqttConfig.PublishTopics.Root, MqttConfig.PublishTopics.Door);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/Alarm/Siren");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        FreeToPublish = false;
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);

        if (memcmp(event->topic, MqttConfig.SubscribeTopics.Signal, event->data_len) == 0)
        {
            if (_xSemaphore == NULL)
                break;

            ESP_LOGI(TAG, "Giving Beep Semaphore");
            xSemaphoreGive(*_xSemaphore);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void Mqtt_PublishValues(uint32_t stamp, float temperature, float humidity, float pressure, int doorOpen)
{
    if (!FreeToPublish)
        return;

    if (_client == NULL)
        return;

    char valueBuf[16];
    cJSON *root;
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "name", cJSON_CreateString(pDeviceConfig->DeviceName));
    cJSON_AddStringToObject(root, "type", "TH");
    sprintf(valueBuf, "%d", stamp);
    cJSON_AddStringToObject(root, "timestamp", valueBuf);
    sprintf(valueBuf, "%.2f °C", temperature);
    cJSON_AddStringToObject(root, "temperature", valueBuf);
    sprintf(valueBuf, "%.2f %%", humidity);
    cJSON_AddStringToObject(root, "humidity", valueBuf);
    sprintf(valueBuf, "%d Pa", (int)pressure);
    cJSON_AddStringToObject(root, "pressure", valueBuf);
    sprintf(valueBuf, "%d", doorOpen);
    cJSON_AddStringToObject(root, "doorOpen", valueBuf);

    char *rendered = cJSON_PrintUnformatted(root); // to save pretty whitespaces cJSON_Print
    cJSON_Delete(root);

    // ESP_LOGI(TAG, "Data: %s", rendered);

    char topicBuf[288];
    sprintf(topicBuf, "%s%s", MqttConfig.PublishTopics.Root, "AllData");

    if (esp_mqtt_client_publish(_client, topicBuf, rendered, 0, 1, 0) == -1)
        ESP_LOGW(TAG, "Sending of 'AllData' failed");
    free(rendered);
}

void Mqtt_PublishTemperature(float temperature)
{
    if (!FreeToPublish)
        return;

    if (_client == NULL)
        return;

    char topicBuf[320]; // todo dependant on definition
    sprintf(topicBuf, "%s%s", MqttConfig.PublishTopics.Root, MqttConfig.PublishTopics.Temperature);
    char valueBuffer[32];
    sprintf(valueBuffer, "%.2f", temperature);

    if (esp_mqtt_client_publish(_client, topicBuf, valueBuffer, 0, 1, 0) == -1)
        ESP_LOGW(TAG, "Sending of 'AllData' failed");
}

void Mqtt_PublishHumidity(float humidity)
{
    if (!FreeToPublish)
        return;

    if (_client == NULL)
        return;

    char topicBuf[320]; // todo dependant on definition
    sprintf(topicBuf, "%s%s", MqttConfig.PublishTopics.Root, MqttConfig.PublishTopics.Humidity);
    char valueBuffer[32];
    sprintf(valueBuffer, "%.2f", humidity);

    if (esp_mqtt_client_publish(_client, topicBuf, valueBuffer, 0, 1, 0) == -1)
        ESP_LOGW(TAG, "Sending of 'AllData' failed");
}

void Mqtt_PublishDoor(int doorOpen)
{
    if (!FreeToPublish)
        return;

    if (_client == NULL)
        return;

    char topicBuf[320]; // todo dependant on definition
    sprintf(topicBuf, "%s%s", MqttConfig.PublishTopics.Root, MqttConfig.PublishTopics.Door);
    char valueBuffer[32];
    sprintf(valueBuffer, "%d", doorOpen);

    if (esp_mqtt_client_publish(_client, topicBuf, valueBuffer, 0, 1, 0) == -1)
        ESP_LOGW(TAG, "Sending of 'AllData' failed");
}

void mqttPublishHealthStatus(float battery, uint32_t runtime)
{
}

void Mqtt_ServiceStart(SemaphoreHandle_t *xSemaphore, MqttConfig_t *mqttCfg, DeviceConfig_t *defCfg)
{
    _xSemaphore = xSemaphore;

    memcpy(&MqttConfig, mqttCfg, sizeof(MqttConfig_t));
    pDeviceConfig = defCfg;

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MqttConfig.BrokerUrl,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    if (esp_mqtt_client_start(client) == ESP_OK)
        _client = client;
    else
        ESP_LOGE(TAG, "Client start failed");
}
