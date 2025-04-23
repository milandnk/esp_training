#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define STORAGE_NAMESPACE "storage"
#define MAX_CHANNEL 10

const char *TAG = "nvs";

typedef struct
{
    uint8_t Channel_Power[MAX_CHANNEL];  /* Power for each channel */
    uint8_t Channel_Type[MAX_CHANNEL];   /* Type of devices */
    uint32_t User_Config_Flag;           /* Config flag for file existence */
    bool Channel_Status[MAX_CHANNEL];    /* Status for each channel */
    uint32_t Aws_Time_Interval;          /* Time delay value of the timer (milliseconds) */
    uint8_t previous_state[MAX_CHANNEL]; /* Store previous state of manual switch */
    uint16_t lum_Err;                    /* Luminance error */
    char device_names[MAX_CHANNEL][16];  /* Names for each channel */
} device_config_t;

esp_err_t print_what_saved(const device_config_t *device_data)
{
    ESP_LOGI(TAG, "Channel_Power[0]: %d", device_data->Channel_Power[0]);
    ESP_LOGI(TAG, "Channel_Type[0]: %d", device_data->Channel_Type[0]);
    ESP_LOGI(TAG, "Channel_Status[0]: %d", device_data->Channel_Status[0]);
    ESP_LOGI(TAG, "Device Name[0]: %s", device_data->device_names[0]);
    return ESP_OK;
}

esp_err_t retrieve_device_info(const char *key_prefix, device_config_t *device)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS handle!");
        return ret;
    }

    // Retrieve the stored blob
    size_t required_size = sizeof(device_config_t);
    ret = nvs_get_blob(nvs_handle, key_prefix, device, &required_size);
    if (ret == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGE(TAG, "No data found in NVS for key: %s", key_prefix);
    }
    else if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to retrieve data from NVS for key: %s", key_prefix);
    }

    nvs_close(nvs_handle);
    return ret;
}

esp_err_t store_data_nvs(const char *key_prefix, const device_config_t *device)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS handle!");
        return ret;
    }

    // Store the entire device structure as a blob
    ret = nvs_set_blob(nvs_handle, key_prefix, device, sizeof(device_config_t));
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to store data in NVS for key: %s", key_prefix);
        nvs_close(nvs_handle);
        return ret;
    }

    ret = nvs_commit(nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit NVS changes!");
    }
    else
    {
        ESP_LOGI(TAG, "Data successfully stored with key: %s", key_prefix);
    }

    nvs_close(nvs_handle);
    return ret;
}

void write_data_nvs(device_config_t *device)
{
    memset(device, 0, sizeof(device_config_t)); // Initialize memory
    device->Channel_Power[0] = 1;
    device->Channel_Type[0] = 11;
    strncpy(device->device_names[0], "milan", sizeof(device->device_names[0]));
    device->Channel_Status[0] = true;

    store_data_nvs("slot_1", device);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    device_config_t device_data;

    write_data_nvs(&device_data);

    memset(&device_data, 0, sizeof(device_config_t)); // Clear the structure
    ESP_LOGI(TAG, "Before retrieving data from NVS:");
    print_what_saved(&device_data);

    retrieve_device_info("slot_1", &device_data);

    ESP_LOGI(TAG, "After retrieving data from NVS:");
    print_what_saved(&device_data);
}
