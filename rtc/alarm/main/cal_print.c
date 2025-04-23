// #include <stdio.h>
// #include <time.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/timers.h"
// #include "esp_log.h"
// #include "driver/gpio.h"
// #include <sys/time.h>  
// #define LED_PIN GPIO_NUM_38

// static const char *TAG = "RTC_ALARM";

// // Set your desired alarm time here (24-hour format)
// #define ALARM_HOUR 17  // 5 PM
// #define ALARM_MINUTE 28 // 02 minutes

// void init_led(void)
// {
//     gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
//     gpio_set_level(LED_PIN, 0); 
// }

// bool check_alarm_time(struct tm *timeinfo)
// {
//     if (timeinfo->tm_hour == ALARM_HOUR && timeinfo->tm_min == ALARM_MINUTE)
//     {
//         return true;
//     }
//     return false;
// }

// void check_time_callback(TimerHandle_t xTimer)
// {
//     time_t now;
//     struct tm timeinfo;

//     time(&now);
//     localtime_r(&now, &timeinfo);

//     ESP_LOGI(TAG, "Current time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

//     if (check_alarm_time(&timeinfo))
//     {
//         ESP_LOGI(TAG, "Alarm triggered! Turning on the LED.");
//         gpio_set_level(LED_PIN, 1);
//         vTaskDelay(pdMS_TO_TICKS(5000)); 

//         gpio_set_level(LED_PIN, 0);
//         ESP_LOGI(TAG, "LED turned off after 5 seconds.");
//     }
// }

// void set_internal_time(struct tm *timeinfo)
// {
//     time_t now = mktime(timeinfo);
//     struct timeval tv = {
//         .tv_sec = now,
//         .tv_usec = 0};
//     settimeofday(&tv, NULL); 
// }

// void app_main(void)
// {
//     init_led(); // Initialize the LED GPIO

//     struct tm initial_time = {
//         .tm_year = 2024 - 1900,
//         .tm_mon = 9 - 1,
//         .tm_mday = 23,
//         .tm_hour = 05,
//         .tm_min = 24,
//         .tm_sec = 0,
//         .tm_isdst = -1 
//     };

//     set_internal_time(&initial_time); 

//     time_t now;
//     struct tm timeinfo;
//     time(&now);
//     localtime_r(&now, &timeinfo);
//     ESP_LOGI(TAG, "Starting time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

//     TimerHandle_t xTimer = xTimerCreate(
//         "check_time_timer",            // Timer name
//         pdMS_TO_TICKS(5000),          // Timer period 
//         pdTRUE,                        // Auto-reload (repeat timer)
//         (void *)0,                     // Timer ID (optional)
//         check_time_callback);          // Callback function

//     if (xTimer != NULL)
//     {
//         xTimerStart(xTimer, 0);
//     }
//     while (1)
//     {
//         ESP_LOGI(TAG, "hii");
//         vTaskDelay(pdMS_TO_TICKS(1000)); 
//     }
    
// }
#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"  // Use esp_timer for hardware timer
#include "esp_log.h"
#include "driver/gpio.h"
#include <sys/time.h>  

#define LED_PIN GPIO_NUM_38

static const char *TAG = "RTC_ALARM";

// Set your desired alarm time here (24-hour format)
#define ALARM_HOUR 17  // 5 PM
#define ALARM_MINUTE 28 // 28 minutes

// Function to initialize the LED
void init_led(void)
{
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0); // Turn off LED initially
}

// Function to check if the current time matches the alarm time
bool check_alarm_time(struct tm *timeinfo)
{
    if (timeinfo->tm_hour == ALARM_HOUR && timeinfo->tm_min == ALARM_MINUTE)
    {
        return true;
    }
    return false;
}

// Timer callback function that checks the current time and triggers the LED
void periodic_timer_callback(void *arg)
{
    time_t now;
    struct tm timeinfo;

    // Get the current time
    time(&now);
    localtime_r(&now, &timeinfo);

    ESP_LOGI(TAG, "Current time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    // Check if the current time matches the alarm time
    if (check_alarm_time(&timeinfo))
    {
        ESP_LOGI(TAG, "Alarm triggered! Turning on the LED.");
        gpio_set_level(LED_PIN, 1);  // Turn on LED
        vTaskDelay(pdMS_TO_TICKS(5000));  // Keep LED on for 5 seconds
        gpio_set_level(LED_PIN, 0);  // Turn off LED
        ESP_LOGI(TAG, "LED turned off after 5 seconds.");
    }
}

// Function to set the initial time (for testing, as there's no NTP here)
void set_internal_time(struct tm *timeinfo)
{
    time_t now = mktime(timeinfo);
    struct timeval tv = {
        .tv_sec = now,
        .tv_usec = 0};
    settimeofday(&tv, NULL);  // Set the system time
}

void app_main(void)
{
    init_led(); // Initialize the LED GPIO

    // Set the initial system time (manually for testing purposes)
    struct tm initial_time = {
        .tm_year = 2024 - 1900,
        .tm_mon = 9 - 1,
        .tm_mday = 23,
        .tm_hour = 17,
        .tm_min = 24,
        .tm_sec = 0,
        .tm_isdst = -1  // No Daylight Saving Time
    };
    set_internal_time(&initial_time);  // Set initial system time

    // Log the starting time
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "Starting time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    // Create a hardware timer using esp_timer to check every 60 seconds
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,  // Callback function
        .name = "periodic_timer"  // Timer name
    };

    esp_timer_handle_t periodic_timer;//create handler
    esp_timer_create(&periodic_timer_args, &periodic_timer);//create timer
    esp_timer_start_periodic(periodic_timer, 5000000);  // Check every 5 seconds (60000000 us)

    // while (1)
    // {
    //     ESP_LOGI(TAG, "Main loop running...");
    //     vTaskDelay(pdMS_TO_TICKS(1000));  // Delay to keep main loop active
    // }
}
