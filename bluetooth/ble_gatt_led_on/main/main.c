#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_nimble_hci.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#define DEVICE_NAME "ESP32_S3_GATT"
#define SERVICE_UUID 0x180F // Battery Service UUID for example
#define CHAR_UUID 0x2A19    // Battery Level Characteristic UUID

#define BLINK_GPIO GPIO_NUM_38

static const char *TAG = "BLE_GATT";

static uint8_t characteristic_value[20] = {0}; // Buffer for characteristic data

static int handle_read(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    ESP_LOGI(TAG, "Read operation on characteristic");
    return os_mbuf_append(ctxt->om, characteristic_value, sizeof(characteristic_value)); // Append characteristic value to the response
}

static int led_blink(uint8_t *str)
{
    uint8_t str1[] = "on";
    uint8_t str2[] = "off";
    if (strcmp((char *)str, (char *)str1) == 0)
    {
        gpio_set_level(BLINK_GPIO, 1);
        ESP_LOGI(TAG, "Red LED on");
    }
    else if (strcmp((char *)str, (char *)str2) == 0)
    {
        gpio_set_level(BLINK_GPIO, 0);
        ESP_LOGI(TAG, "LED off");
    }
    return 0;
}

static int handle_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    uint16_t len = os_mbuf_len(ctxt->om); // Get the length of incoming data
    if (len > sizeof(characteristic_value) - 1)
    {
        ESP_LOGE(TAG, "Write operation failed: Invalid length");
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN; // Return appropriate error code
    }

    int rc = os_mbuf_copydata(ctxt->om, 0, len, characteristic_value);
    if (rc == 0)
    {
        characteristic_value[len] = '\0'; // Null-terminate the string
        ESP_LOGI(TAG, "Write operation on characteristic: %.*s", len, characteristic_value);
        led_blink(characteristic_value);
    }
    else
    {
        ESP_LOGE(TAG, "Error writing characteristic");
        return BLE_ATT_ERR_UNLIKELY;
    }
    return 0;
}

static int on_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    switch (ctxt->op)
    {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        return handle_read(conn_handle, attr_handle, ctxt, arg);
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        return handle_write(conn_handle, attr_handle, ctxt, arg);
    default:
        return BLE_ATT_ERR_UNLIKELY;
    }
}

static const struct ble_gatt_svc_def gatt_svr_services[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(SERVICE_UUID),
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = BLE_UUID16_DECLARE(CHAR_UUID),
                .access_cb = on_access,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                .min_key_size = 0,
            },
            {
                0, // No more characteristics in this service
            },
        },
    },
    {
        0, // No more services
    },
};

static int ble_event_handler(struct ble_gap_event *event, void *arg);

// Function to start advertising
static void start_advertising(void)
{
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));

    // Set advertisement flags
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    // Set the complete device name
    fields.name = (uint8_t *)DEVICE_NAME;
    fields.name_len = strlen(DEVICE_NAME);
    fields.name_is_complete = 1;

    // Set the advertising data
    int rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Setting advertisement data failed: %d", rc);
        return;
    }

    // Start advertising
    rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, ble_event_handler, NULL);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Advertising start failed: %d", rc);
    }
    else
    {
        ESP_LOGI(TAG, "Advertising started");
    }
}

static int ble_event_handler(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "Connection established");
        if (event->connect.status != 0)
        {
            start_advertising(); // Restart advertising if connection failed
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Disconnected, restarting advertising");
        start_advertising();
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "Advertising complete, restarting advertising");
        start_advertising();
        break;
    default:
        break;
    }
    return 0;
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize GPIO
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BLINK_GPIO, 0); // Ensure the LED is initially off

    // Initialize NimBLE stack
    nimble_port_init();
    ble_hs_cfg.sync_cb = start_advertising;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    // Initialize the GATT server
    ret = ble_gatts_count_cfg(gatt_svr_services);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "Failed to count GATT services: %d", ret);
        return;
    }
    ret = ble_gatts_add_svcs(gatt_svr_services);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "Failed to add GATT services: %d", ret);
        return;
    }

    // Start the NimBLE task
    nimble_port_freertos_init(nimble_port_run);

    ESP_LOGI(TAG, "BLE GATT Server Started");
}
