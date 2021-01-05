/**
 * @file SimpleServer.c
 * @author Gustice
 * @brief Small webserver to configure the MQTT-Device and WiFi-Paramter
 * @details Derived from Espressif-Example "\examples\protocols\http_server\simple"
 * @version 0.1
 * @date 2021-01-01
 * 
 * @copyright Copyright (c) 2021
 */

#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "WifiConnect.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "cJSON.h"

#include "SimpleServer.h"
#include "ParamRepo.h"

static const char *TAG = "WebServer";

extern const unsigned char mainPage_html_start[] asm("_binary_mainPage_html_start");
extern const unsigned char mainPage_html_end[] asm("_binary_mainPage_html_end");

DeviceConfig_t *deviceConfig;
MqttConfig_t *mqttConfig;

/* An HTTP GET handler */
esp_err_t page_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Page request");
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)mainPage_html_start, mainPage_html_end - mainPage_html_start);

    return ESP_OK;
}

httpd_uri_t page = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = page_get_handler,
    .user_ctx = ""};

esp_err_t data_get_handler(httpd_req_t *req)
{
    cJSON *root;
    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", deviceConfig->DeviceName);
    cJSON_AddStringToObject(root, "location", deviceConfig->Location);

    cJSON_AddStringToObject(root, "broker", mqttConfig->BrokerUrl);
    cJSON_AddStringToObject(root, "rootTopic", mqttConfig->PublishTopics.Root);
    cJSON_AddStringToObject(root, "tempTopic", mqttConfig->PublishTopics.Temperature);
    cJSON_AddStringToObject(root, "humTopic", mqttConfig->PublishTopics.Humidity);
    cJSON_AddStringToObject(root, "doorTopic", mqttConfig->PublishTopics.Door);

    cJSON_AddStringToObject(root, "promptTopic", mqttConfig->SubscribeTopics.Prompt);
    cJSON_AddStringToObject(root, "signalTopic", mqttConfig->SubscribeTopics.Signal);

    char *rendered = cJSON_PrintUnformatted(root); // to save pretty whitespaces cJSON_Print
    cJSON_Delete(root);
    ESP_LOGI(TAG, "Data: %s", rendered);

    httpd_resp_set_type(req, "text/json");
    httpd_resp_send(req, rendered, -1);

    free(rendered);
    return ESP_OK;
}

httpd_uri_t data = {
    .uri = "/setup",
    .method = HTTP_GET,
    .handler = data_get_handler,
    .user_ctx = ""};

/* An HTTP POST handler */
esp_err_t set_post_handler(httpd_req_t *req)
{
    char buf[254];
    int total_len = req->content_len;
    const int BufferSize = sizeof(buf);

    int cur_len = 0;
    int received = 0;
    if (total_len >= BufferSize)
    {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_500(req); // content too long
        return ESP_FAIL;
    }

    while (cur_len < total_len)
    {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0)
        {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_500(req); // Failed to post control value
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    /* Log data received */
    ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
    ESP_LOGI(TAG, "%s", buf);
    ESP_LOGI(TAG, "====================================");

    cJSON *root = cJSON_Parse(buf);
    const char *command = cJSON_GetObjectItem(root, "cmd")->valuestring;

    if (strcmp(command, "wifiCfg") == 0)
    {
        ESP_LOGI(TAG, "WiFi-Config Request");

        WifiConfig_t wifiCfg;
        memset(&wifiCfg, 0, sizeof(WifiConfig_t));

        const char *ssid = cJSON_GetObjectItem(root, "ssid")->valuestring;
        const char *password = cJSON_GetObjectItem(root, "password")->valuestring;

        strcpy((char *)wifiCfg.ssid, ssid);
        strcpy((char *)wifiCfg.password, password);

        Fs_SaveEntry(wifiCfgFile, (void *)&wifiCfg, sizeof(WifiConfig_t));
    }
    else if (strcmp(command, "locCfg") == 0)
    {
        ESP_LOGI(TAG, "Device-Config Request");

        DeviceConfig_t devCfg;
        memset(&devCfg, 0, sizeof(DeviceConfig_t));

        const char *name = cJSON_GetObjectItem(root, "name")->valuestring;
        const char *location = cJSON_GetObjectItem(root, "location")->valuestring;

        strcpy((char *)devCfg.DeviceName, name);
        strcpy((char *)devCfg.Location, location);

        Fs_SaveEntry(devCfgFile, (void *)&devCfg, sizeof(DeviceConfig_t));
    }
    else if (strcmp(command, "mqttCfg") == 0)
    {
        ESP_LOGI(TAG, "MQTT-Config Request");

        MqttConfig_t mqttCfg;
        memset(&mqttCfg, 0, sizeof(MqttConfig_t));

        const char *broker = cJSON_GetObjectItem(root, "broker")->valuestring;
        const char *rt = cJSON_GetObjectItem(root, "rootTopic")->valuestring;
        const char *temp = cJSON_GetObjectItem(root, "tempTopic")->valuestring;
        const char *hum = cJSON_GetObjectItem(root, "humTopic")->valuestring;
        const char *bin = cJSON_GetObjectItem(root, "binTopic")->valuestring;
        const char *signal = cJSON_GetObjectItem(root, "signalTopic")->valuestring;
        const char *prompt = cJSON_GetObjectItem(root, "promptTopic")->valuestring;

        strcpy((char *)mqttCfg.BrokerUrl, broker);
        strcpy((char *)mqttCfg.PublishTopics.Root, rt);
        strcpy((char *)mqttCfg.PublishTopics.Temperature, temp);
        strcpy((char *)mqttCfg.PublishTopics.Humidity, hum);
        strcpy((char *)mqttCfg.PublishTopics.Door, bin);
        strcpy((char *)mqttCfg.SubscribeTopics.Signal, signal);
        strcpy((char *)mqttCfg.SubscribeTopics.Prompt, prompt);

        Fs_SaveEntry(mqttCfgFile, (void *)&mqttCfg, sizeof(MqttConfig_t));
    }
    else
    {
        /* code */
    }

    cJSON_Delete(root);
    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

httpd_uri_t setup = {
    .uri = "/setup",
    .method = HTTP_POST,
    .handler = set_post_handler,
    .user_ctx = NULL};

void Web_StartWebserver(DeviceConfig_t *devCfg, MqttConfig_t *mqttCfg)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    deviceConfig = devCfg;
    mqttConfig = mqttCfg;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &page);
        httpd_register_uri_handler(server, &data);
        httpd_register_uri_handler(server, &setup);
        return;
    }

    ESP_LOGI(TAG, "Error starting server!");
}
