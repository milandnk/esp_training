#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#include "esp_bt_device.h"

#define TAG "BLE_SPP_SIM"

// UUIDs for the service and characteristic
#define SPP_SERVICE_UUID        0x00FF
#define SPP_CHAR_UUID           0xFF01

// GATT attributes
static uint8_t spp_char_value[512] = {0};
static esp_attr_value_t gatts_spp_char_val = {
    .attr_max_len = sizeof(spp_char_value),
    .attr_len     = sizeof(spp_char_value),
    .attr_value   = spp_char_value,
};

static esp_gatt_char_prop_t spp_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

static uint16_t spp_handle_table[2]; // [0] for service handle, [1] for characteristic handle

// Event handler for GATT server events
static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
        case ESP_GATTS_REG_EVT:
            ESP_LOGI(TAG, "Service registered, starting creation");
            esp_gatt_srvc_id_t service_id = {
                .is_primary = true,
                .id = {
                    .inst_id = 0,
                    .uuid = {
                        .len = ESP_UUID_LEN_16,
                        .uuid = {
                            .uuid16 = SPP_SERVICE_UUID,
                        },
                    },
                },
            };
            esp_ble_gatts_create_service(gatts_if, &service_id, 4);
            break;
        case ESP_GATTS_CREATE_EVT:
            ESP_LOGI(TAG, "Service created, adding characteristic");
            spp_handle_table[0] = param->create.service_handle;
            esp_bt_uuid_t char_uuid = {
                .len = ESP_UUID_LEN_16,
                .uuid = {
                    .uuid16 = SPP_CHAR_UUID,
                },
            };
            esp_ble_gatts_add_char(spp_handle_table[0], &char_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, spp_property, &gatts_spp_char_val, NULL);
            break;
        case ESP_GATTS_ADD_CHAR_EVT:
            ESP_LOGI(TAG, "Characteristic added");
            spp_handle_table[1] = param->add_char.attr_handle;
            esp_ble_gatts_start_service(spp_handle_table[0]);
            break;
        default:
            break;
    }
}

void app_main(void) {
    esp_err_t ret;

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        return;
    }

    // Initialize Bluetooth
    esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    // Initialize GATT server
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_profile_event_handler));
}
