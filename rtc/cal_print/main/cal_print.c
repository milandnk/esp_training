#include <stdio.h>
#include <time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_system.h"
#include "sys/time.h"

static const char *TAG = "CALENDAR_APP";

void set_internal_time(struct tm *timeinfo)
{
    // Convert tm structure to time_t (seconds since epoch)
    time_t now = mktime(timeinfo);

    // Set the time using the internal RTC
    struct timeval tv = {
        .tv_sec = now,
        .tv_usec = 0};
    settimeofday(&tv, NULL); // set time
}

// Function to print the current date and time
void print_datetime(void)
{
    struct timeval tv;
    struct tm timeinfo;

    gettimeofday(&tv, NULL);

    // Convert to local timet to tm
    localtime_r(&tv.tv_sec, &timeinfo);

    // Format and print the date and time
    // strftime(strftime_buf, sizeof(strftime_buf), "%A, %B %d %Y %H:%M:%S", &timeinfo);//for stored
    ESP_LOGI(TAG, "Current time: %04d-%02d-%02d %02d:%02d:%02d",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
             timeinfo.tm_mday, timeinfo.tm_hour,
             timeinfo.tm_min, timeinfo.tm_sec);
}

// Main task
void calendar_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Calendar Application started...");

    struct tm initial_time = {
        // init time
        .tm_year = 2024 - 1900,
        .tm_mon = 9 - 1, // 0 to ....
        .tm_mday = 23,
        .tm_hour = 02,
        .tm_min = 30,
        .tm_sec = 0,
        .tm_isdst = -1 // No Daylight Saving Time
    };

    set_internal_time(&initial_time); // set init time

    while (1)
    {
        print_datetime();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main()
{
    xTaskCreate(calendar_task, "calendar_task", 1024 * 8, NULL, 5, NULL);
}