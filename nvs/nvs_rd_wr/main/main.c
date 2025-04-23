#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"

#define STORAGE_NAMESPACE "storage"

// Structure definition for blob data
struct data
{
    char data[32]; // Fixed-size buffer for storing strings
};


esp_err_t print_what_saved(void)
{
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    char key[16];
    char value[32];
    // Read the restart counter
    int32_t restart_counter = 0;
    size_t required_size = 0;
    for (int i = 1; i <= 3; i++)
    {
        snprintf(key, sizeof(key), "slot_%d", i);
        required_size = sizeof(value);
        err = nvs_get_blob(my_handle, key, value, &required_size);
        if (err == ESP_OK)
        {
            printf("%s: %s\n", key, value);
        }
        else if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            printf("%s is Not Found\n", key);
        }
        else
        {
            printf("Error reading %s: %s\n", key, esp_err_to_name(err));
        }
    }

    nvs_close(my_handle);
    return ESP_OK;
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Create and initialize data structures
    struct data d1[3] = {0};
    strcpy(d1[0].data, "data_1");
    strcpy(d1[1].data, "data_2");
    strcpy(d1[2].data, "data_3");


    nvs_handle_t my_handle;
    ESP_ERROR_CHECK(nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_set_blob(my_handle, "slot_1", d1[0].data, strlen(d1[0].data) + 1));
    ESP_ERROR_CHECK(nvs_set_blob(my_handle, "slot_2", d1[1].data, strlen(d1[1].data) + 1));
    ESP_ERROR_CHECK(nvs_set_blob(my_handle, "slot_3", d1[2].data, strlen(d1[2].data) + 1));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
    printf("////////////////////////first time//////////////////////////\n");
    ESP_ERROR_CHECK(print_what_saved());

    ESP_ERROR_CHECK(nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle));
    ESP_ERROR_CHECK(nvs_erase_key(my_handle, "slot_2"));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);
    printf("////////////////////////after slot 2 erase//////////////////////////\n");
    ESP_ERROR_CHECK(print_what_saved());

    ESP_ERROR_CHECK(nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle));
    strcpy(d1[1].data, "data_2_second_time");
    ESP_ERROR_CHECK(nvs_set_blob(my_handle, "slot_2", d1[1].data, strlen(d1[1].data) + 1));
    ESP_ERROR_CHECK(nvs_commit(my_handle));
    nvs_close(my_handle);

    printf("////////////////////////after slot 2 write//////////////////////////\n");
    ESP_ERROR_CHECK(print_what_saved());
    // Infinite loop
    while (1)
    {
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
