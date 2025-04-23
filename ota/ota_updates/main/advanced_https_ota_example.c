#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "OTA"
#define FIRMWARE_URL "http://192.168.30.103:8000/ota_updates.bin" // URL for the next update

void print_version_info()
{
    const esp_app_desc_t *app_desc = esp_ota_get_app_description();
    ESP_LOGI(TAG, "Project Name: %s", app_desc->project_name);
    ESP_LOGI(TAG, "App Version: %s", app_desc->version);
    ESP_LOGI(TAG, "Compile Time: %s %s", app_desc->date, app_desc->time);
    ESP_LOGI(TAG, "ESP-IDF Version: %s", app_desc->idf_ver);
}

void perform_ota_update()
{
    esp_http_client_config_t config = {
        .url = FIRMWARE_URL,
        .timeout_ms = 5000,
        .skip_cert_common_name_check = true // For development purposes
    };

    esp_https_ota_handle_t ota_handle;
    esp_err_t ret = esp_https_ota_begin(&config, &ota_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "OTA begin failed: %s", esp_err_to_name(ret));
        return;
    }

    esp_app_desc_t new_app_desc;
    ret = esp_https_ota_get_img_desc(ota_handle, &new_app_desc);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get new image description: %s", esp_err_to_name(ret));
        esp_https_ota_abort(ota_handle);
        return;
    }

    const esp_app_desc_t *running_app_desc = esp_ota_get_app_description();
    ESP_LOGI(TAG, "Current Version: %s", running_app_desc->version);
    ESP_LOGI(TAG, "New Version: %s", new_app_desc.version);

    if (strcmp(new_app_desc.version, running_app_desc->version) == 0)
    {
        ESP_LOGW(TAG, "Already running the latest version.");
        esp_https_ota_abort(ota_handle);
        return;
    }

    while (true)
    {
        ret = esp_https_ota_perform(ota_handle);
        if (ret != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
        {
            break;
        }
        vTaskDelay(500);
    }

    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "OTA update successful, restarting...");
        esp_https_ota_finish(ota_handle);
        esp_restart();
    }
    else
    {
        ESP_LOGE(TAG, "OTA update failed: %s", esp_err_to_name(ret));
        esp_https_ota_abort(ota_handle);
    }
}

void app_main()
{
    ESP_LOGI(TAG, "This is Version 2 firmware.");
    print_version_info();
    perform_ota_update();
}
