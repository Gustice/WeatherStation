/**
 * @file SntpProcessor.c
 * @author Gustice
 * @brief Simple Network Time Protocol task.
 * @details Derived from Espressif-Example "\examples\protocols\sntp" 
 *  and modified to meet local time
 * @note Currently not really usable 
 * @version 0.1
 * @date 2021-01-01
 * 
 * @copyright Copyright (c) 2021
 * 
 * @todo: Synchronize local time ocasionally with network time.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "SntpProcessor.h"
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

static const char *TAG = "sntp_example";

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

static void obtain_time(void)
{
    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;

    while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

void sntp_example_task(void *arg)
{
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];

    time(&now);
    localtime_r(&now, &timeinfo);

    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900))
    {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
    }

    // Set timezone to Eastern Standard Time and print local time
    // setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    // tzset();

    // Set timezone to "Central European Standard Time"
    setenv("TZ", "CEST-1", 1); // https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
    tzset();

    while (1)
    {
        // update 'now' variable with current time
        time(&now);
        localtime_r(&now, &timeinfo);

        if (timeinfo.tm_year < (2016 - 1900))
        {
            ESP_LOGE(TAG, "The current date/time error");
        }
        else
        {
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
            ESP_LOGI(TAG, "The current date/time in Germany is: %s", strftime_buf);
        }

        ESP_LOGI(TAG, "Free heap size: %d\n", esp_get_free_heap_size());
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void Sntp_Start(void)
{
    xTaskCreate(sntp_example_task, "sntp_example_task", 2048, NULL, 10, NULL);
}
