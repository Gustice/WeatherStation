/**
 * @file WeatherStation.cpp 
 * @author Gustice
 * @brief Simple MQTT-Weather-Station based on Espressif IDF
 * @version 0.1
 * @date 2021-01-01
 * 
 * @copyright Copyright (c) 2021
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "WifiConnect.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lwip/apps/sntp.h"

#include "SntpProcessor.h"
#include "SimpleServer.h"
#include "MqttDevice.h"
#include "I2cPort.h"
#include "SpiPort.h"
#include "Shtc3_Sensor.h"
#include "Display_SSD1306.h"
#include "GpioPort.h"
#include "MyDisplay.h"
#include "ParamRepo.h"

static const char *TAG = "main";
static const char * StdStationSsid = CONFIG_AP_MODE_WIFI_SSID;
static const char * StdStationPw = CONFIG_AP_MODE__WIFI_PASSWORD;

factoryInfo_t factoryCfg;

WifiConfig_t wifiCfg;
DeviceConfig_t devCfg;
MqttConfig_t mqttCfg;

extern "C"
{ // This switch allows the ROS C-implementation to find this main
    void app_main(void);
}


static SemaphoreHandle_t xSemaphore = NULL;
static void AlarmTask(void *pvParameters)
{
    if (xSemaphore == NULL)
    {
        ESP_LOGE(TAG, "Semaphore missing");
        while (true)
        {
            vTaskDelay(20000 / portTICK_RATE_MS);
        }
    }

    while (true)
    {
        if (xSemaphoreTake(xSemaphore, 0) == pdTRUE)
        {
            ESP_LOGI(TAG, "Beep Triggered");
            // Todo Do some awesome action here
            vTaskDelay(200 / portTICK_RATE_MS);
        }
        vTaskDelay(20 / portTICK_RATE_MS);
    }
}

void waitForConfigurationThenRestart(void)
{
    ESP_LOGW(TAG, "No WiFi-Device configuration. Waiting for valid configuration ...");
    WifiConfig_t stationCfg;
    memset(&stationCfg, 0, sizeof(WifiConfig_t));
    memcpy(stationCfg.ssid, StdStationSsid, strlen(StdStationSsid));
    memcpy(stationCfg.password, StdStationPw, strlen(StdStationPw));
    ESP_ERROR_CHECK(Wifi_Connect(InitAsAp, &stationCfg ));
    start_webserver(); 
    while (true) // @todo Semaphore for Reset
    {
        vTaskDelay(2000 / portTICK_RATE_MS);
        // Wait for Access-Point configuration
    }
    ESP_LOGI(TAG, "Restarting ...");
    fflush(stdout);
    esp_restart();
}

void waitForDeviceSetting(void)
{
    ESP_LOGW(TAG, "No Mqtt-Device configuration. Waiting for valid configuration ...");
    while (Fs_CheckIfExists(mqttCfgFile) != ESP_OK) 
    {
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
    ESP_LOGI(TAG, "Configuration found");
}

void app_main()
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    Fs_SetupSpiFFs();

    Fs_ReadFactoryConfiguration(&factoryCfg);
    ESP_LOGI(TAG, "Read Factory Config:\n\tSN: %s\n\tHW: %s\n\tDev: %s", 
    factoryCfg.SerialNumber, factoryCfg.HwVersion, factoryCfg.DeviceType );

    InputPort rstConfig(GPIO_NUM_16); //D0
    if (rstConfig.ReadPort() == 0)
    {
        //displayUi.PrintLogLine("Resetting config"); 
        ESP_LOGW(TAG, "Resetting Wifi and Mqtt config");
        Fs_DeleteEntry(wifiCfgFile);
        Fs_DeleteEntry(mqttCfgFile);
    }

    if (Fs_CheckIfExists(wifiCfgFile) != ESP_OK)
        waitForConfigurationThenRestart();

    Fs_ReadEntry(wifiCfgFile, (void *)&wifiCfg, sizeof(WifiConfig_t) );
    ESP_ERROR_CHECK(Wifi_Connect(ConnectToAp, &wifiCfg));
    start_webserver(); 

    if (Fs_CheckIfExists(mqttCfgFile) != ESP_OK)
        waitForDeviceSetting();

    Fs_ReadEntry(mqttCfgFile, (void *)&mqttCfg, sizeof(MqttConfig_t) );
    xSemaphore = xSemaphoreCreateBinary();

    ESP_LOGI(TAG, "Setup Hardware");
    I2cPort sensPort; // Note: static is somehow not allowed here ?!?
    SpiPort displayPort(mode0);
    OutputPort dispDc(GPIO_NUM_2); // D4
    OutputPort dispRst(GPIO_NUM_0); // D3
    InputPort doorSwitch(GPIO_NUM_15); // D8

    vTaskDelay(1 / portTICK_PERIOD_MS);

    ESP_LOGD(TAG, "Setup Sensor");
    Shtc3_Sensor tempSens(&sensPort);
    ESP_LOGD(TAG, "Setup Display");
    SSD1306 displayDriver(&displayPort, &dispDc, &dispRst);
    ESP_LOGD(TAG, "Setup Ui");
    MyDisplay displayUi(&displayDriver);
    displayUi.PrintInitFrame(true,true,0.5f);

    ESP_LOGD(TAG, "Reading Sensor ID");
    uint16_t sensId; 
    if (tempSens.ReadID(&sensId) == ESP_OK)
        ESP_LOGI(TAG, "T/H-Sensor-ID: %d", sensId);
    else
        ESP_LOGW(TAG, "Sensor ID could not be determined");
    
    //Sntp_Start();
    Mqtt_ServiceStart(&xSemaphore, &mqttCfg, &devCfg);
    xTaskCreate(AlarmTask, "alarmTask", 4096, NULL, 5, NULL);

    uint32_t cycleStamp = 0;
    float temp; 
    float hum;
    int doorOpen;
    char line[32];
    int timeout;

    while (true)
    {
        timeout = 3;
        esp_err_t ret; 

        do {
            ret = tempSens.ReadSensor(&temp, &hum);
        } while (ret != ESP_OK && timeout-- >= 0 );
        doorOpen = doorSwitch.ReadPort();
        
        if ( ret != ESP_OK)
        {
            displayUi.PrintLogLine(line);
            ESP_LOGW(TAG, "Sensor-read failed");
        }
        else
        {
            displayUi.PrintLogLine(line);
            ESP_LOGI(TAG, "T=%d / H=%d", (uint32_t)temp, (uint32_t)hum);
            Mqtt_PublishValues(cycleStamp, temp, 0.5f, 0, doorOpen);
            Mqtt_PublishTemperature(temp);
            Mqtt_PublishHumidity(hum);
            Mqtt_PublishDoor(doorOpen);
        }
                
        cycleStamp++;
        vTaskDelay(30000 / portTICK_RATE_MS);
    }
}
