// #include <stdio.h>
// #include <string.h>
// #include <time.h>
// #include <sys/time.h>
// #include <esp_wifi.h>
// #include <esp_event.h>
// #include <esp_log.h>
// #include <nvs_flash.h>
// #include <esp_sntp.h>

// #define WIFI_SSID "ABCD"
// #define WIFI_PASS "00000000"

// static const char *TAG = "sntp_example";

// static void initialize_sntp(void)
// {
//     ESP_LOGI(TAG, "Initializing SNTP");
//     sntp_setoperatingmode(SNTP_OPMODE_POLL);
//     sntp_setservername(0, "pool.ntp.org");
//     sntp_init();
// }

// static void obtain_time(void)
// {
//     initialize_sntp();

//     // Wait for time to be set
//     time_t now = 0;
//     struct tm timeinfo = {0};
//     int retry = 0;
//     const int retry_count = 10;
//     while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count)
//     {
//         ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
//         vTaskDelay(2000 / portTICK_PERIOD_MS);
//         time(&now);
//         localtime_r(&now, &timeinfo);
//     }

//     if (retry == retry_count)
//     {
//         ESP_LOGE(TAG, "Failed to get time from SNTP server");
//     }

//     // Set timezone (IST)
//     setenv("TZ", "IST-5:30", 1);
//     tzset();
// }

// static void log_time(void *arg)
// {
//     time_t now;
//     struct tm timeinfo;

//     while (1)
//     {
//         time(&now);
//         localtime_r(&now, &timeinfo);
//         ESP_LOGI(TAG, "Current time: %s", asctime(&timeinfo));
//         vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
//     }
// }

// static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
// {
//     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
//     {
//         esp_wifi_connect();
//     }
//     else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
//     {
//         esp_wifi_connect();
//     }
//     else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
//     {
//         obtain_time();
//         xTaskCreate(log_time, "log_time", 4096, NULL, 5, NULL);
//     }
// }

// void wifi_init_sta(void)
// {
//     esp_netif_init();
//     esp_event_loop_create_default();
//     esp_netif_create_default_wifi_sta();
//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     esp_wifi_init(&cfg);

//     esp_event_handler_instance_t instance_any_id;
//     esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
//     esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_any_id);

//     wifi_config_t wifi_config = {
//         .sta = {
//             .ssid = WIFI_SSID,
//             .password = WIFI_PASS,
//         },
//     };

//     esp_wifi_set_mode(WIFI_MODE_STA);
//     esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
//     esp_wifi_start();
// }

// void app_main(void)
// {
//     ESP_ERROR_CHECK(nvs_flash_init());
//     wifi_init_sta();
// }


#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_sntp.h>

#define WIFI_SSID "ABCD"
#define WIFI_PASS "00000000"

static const char *TAG = "sntp_example";

void set_manual_time(void)
{
    struct tm timeinfo = {
        .tm_year = 2025 - 1900, // Year since 1900 (e.g., 2025 - 1900)
        .tm_mon = 0,            // Month (0 = January)
        .tm_mday = 27,          // Day of the month
        .tm_hour = 15,          // Hour (24-hour format)
        .tm_min = 21,           // Minute
        .tm_sec = 0             // Second
    };

    time_t t = mktime(&timeinfo); // Convert to time_t
    if (t == -1)
    {
        ESP_LOGE(TAG, "Failed to set manual time.");
        return;
    }

    struct timeval now = {
        .tv_sec = t,
        .tv_usec = 0
    };

    if (settimeofday(&now, NULL) != 0)
    {
        ESP_LOGE(TAG, "Failed to set system time.");
    }
    else
    {
        ESP_LOGI(TAG, "Manual time set: %s", asctime(&timeinfo));
    }
}

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

    // Wait for time to be set
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

    if (retry == retry_count)
    {
        ESP_LOGW(TAG, "Failed to get time from SNTP server. Setting manual time.");
        set_manual_time(); // Call the manual time-setting function
    }

    // Set timezone (IST)
    setenv("TZ", "IST-5:30", 1);
    tzset();
}

static void log_time(void *arg)
{
    time_t now;
    struct tm timeinfo;

    while (1)
    {
        time(&now);
        localtime_r(&now, &timeinfo);
        ESP_LOGI(TAG, "Current time: %s", asctime(&timeinfo));
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        obtain_time();
        xTaskCreate(log_time, "log_time", 4096, NULL, 5, NULL);
    }
}

void wifi_init_sta(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_any_id);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
}
