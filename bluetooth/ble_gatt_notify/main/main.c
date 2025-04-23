// // #include <stdio.h>
// // #include <string.h>
// // #include "esp_log.h"
// // #include "driver/gpio.h"
// // #include "nvs_flash.h"
// // #include "esp_nimble_hci.h"
// // #include "host/ble_hs.h"
// // #include "host/ble_uuid.h"
// // #include "host/util/util.h"
// // #include "nimble/ble.h"
// // #include "nimble/nimble_port.h"
// // #include "nimble/nimble_port_freertos.h"

// // #define DEVICE_NAME "ESP32_S3_GATT"
// // #define SERVICE_UUID 0x180F // Battery Service UUID for example
// // #define CHAR_UUID 0x2A19    // Battery Level Characteristic UUID

// // #define BLINK_GPIO GPIO_NUM_38

// // static const char *TAG = "BLE_GATT";

// // static uint8_t characteristic_value[200] = {0}; // Buffer for characteristic data

// // static int handle_read(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
// // {
// //     ESP_LOGI(TAG, "Read operation on characteristic");
// //     send_data(characteristic_value);
// //     return os_mbuf_append(ctxt->om, characteristic_value, sizeof(characteristic_value)); // Append characteristic value to the response
// // }

// // static int send_data(uint8_t *str)
// // {
// //     for (int i = 0; i < 200; i++)
// //     {
// //         str[i] = i;
// //     }
// // }

// // static int handle_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
// // {
// //     uint16_t len = os_mbuf_len(ctxt->om); // Get the length of incoming data
// //     if (len > sizeof(characteristic_value) - 1)
// //     {
// //         ESP_LOGE(TAG, "Write operation failed: Invalid length");
// //         return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN; // Return appropriate error code
// //     }

// //     int rc = os_mbuf_copydata(ctxt->om, 0, len, characteristic_value);
// //     if (rc == 0)
// //     {
// //         characteristic_value[len] = '\0'; // Null-terminate the string
// //         ESP_LOGI(TAG, "Write operation on characteristic: %.*s", len, characteristic_value);
// //     }
// //     else
// //     {
// //         ESP_LOGE(TAG, "Error writing characteristic");
// //         return BLE_ATT_ERR_UNLIKELY;
// //     }
// //     return 0;
// // }

// // static int on_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
// // {
// //     switch (ctxt->op)
// //     {
// //     case BLE_GATT_ACCESS_OP_READ_CHR:
// //         return handle_read(conn_handle, attr_handle, ctxt, arg);
// //     case BLE_GATT_ACCESS_OP_WRITE_CHR:
// //         return handle_write(conn_handle, attr_handle, ctxt, arg);
// //     default:
// //         return BLE_ATT_ERR_UNLIKELY;
// //     }
// // }

// // static const struct ble_gatt_svc_def gatt_svr_services[] = {
// //     {
// //         .type = BLE_GATT_SVC_TYPE_PRIMARY,
// //         .uuid = BLE_UUID16_DECLARE(SERVICE_UUID),
// //         .characteristics = (struct ble_gatt_chr_def[]){
// //             {
// //                 .uuid = BLE_UUID16_DECLARE(CHAR_UUID),
// //                 .access_cb = on_access,
// //                 .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
// //                 .min_key_size = 0,
// //             },
// //             {
// //                 0, // No more characteristics in this service
// //             },
// //         },
// //     },
// //     {
// //         0, // No more services
// //     },
// // };

// // static int ble_event_handler(struct ble_gap_event *event, void *arg);

// // // Function to start advertising
// // static void start_advertising(void)
// // {
// //     struct ble_gap_adv_params adv_params;
// //     memset(&adv_params, 0, sizeof(adv_params));
// //     adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
// //     adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

// //     struct ble_hs_adv_fields fields;
// //     memset(&fields, 0, sizeof(fields));

// //     // Set advertisement flags
// //     fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

// //     // Set the complete device name
// //     fields.name = (uint8_t *)DEVICE_NAME;
// //     fields.name_len = strlen(DEVICE_NAME);
// //     fields.name_is_complete = 1;

// //     // Set the advertising data
// //     int rc = ble_gap_adv_set_fields(&fields);
// //     if (rc != 0)
// //     {
// //         ESP_LOGE(TAG, "Setting advertisement data failed: %d", rc);
// //         return;
// //     }

// //     // Start advertising
// //     rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, ble_event_handler, NULL);
// //     if (rc != 0)
// //     {
// //         ESP_LOGE(TAG, "Advertising start failed: %d", rc);
// //     }
// //     else
// //     {
// //         ESP_LOGI(TAG, "Advertising started");
// //     }
// // }

// // static int ble_event_handler(struct ble_gap_event *event, void *arg)
// // {
// //     switch (event->type)
// //     {
// //     case BLE_GAP_EVENT_CONNECT:
// //         ESP_LOGI(TAG, "Connection established");
// //         if (event->connect.status != 0)
// //         {
// //             start_advertising(); // Restart advertising if connection failed
// //         }
// //         break;
// //     case BLE_GAP_EVENT_DISCONNECT:
// //         ESP_LOGI(TAG, "Disconnected, restarting advertising");
// //         start_advertising();
// //         break;
// //     case BLE_GAP_EVENT_ADV_COMPLETE:
// //         ESP_LOGI(TAG, "Advertising complete, restarting advertising");
// //         start_advertising();
// //         break;
// //     default:
// //         break;
// //     }
// //     return 0;
// // }

// // void app_main(void)
// // {
// //     // Initialize NVS
// //     esp_err_t ret = nvs_flash_init();
// //     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
// //     {
// //         ESP_ERROR_CHECK(nvs_flash_erase());
// //         ret = nvs_flash_init();
// //     }
// //     ESP_ERROR_CHECK(ret);

// //     // Initialize GPIO
// //     gpio_reset_pin(BLINK_GPIO);
// //     gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
// //     gpio_set_level(BLINK_GPIO, 0); // Ensure the LED is initially off

// //     // Initialize NimBLE stack
// //     nimble_port_init();
// //     ble_hs_cfg.sync_cb = start_advertising;
// //     ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

// //     // Initialize the GATT server
// //     ret = ble_gatts_count_cfg(gatt_svr_services);
// //     if (ret != 0)
// //     {
// //         ESP_LOGE(TAG, "Failed to count GATT services: %d", ret);
// //         return;
// //     }
// //     ret = ble_gatts_add_svcs(gatt_svr_services);
// //     if (ret != 0)
// //     {
// //         ESP_LOGE(TAG, "Failed to add GATT services: %d", ret);
// //         return;
// //     }

// //     // Start the NimBLE task
// //     nimble_port_freertos_init(nimble_port_run);

// //     ESP_LOGI(TAG, "BLE GATT Server Started");
// // }

// #include <stdio.h>
// #include <stdlib.h> // for random value generation
// #include <string.h> // for strlen and snprintf
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"
// #include "freertos/event_groups.h"
// #include "esp_event.h"
// #include "nvs_flash.h"
// #include "esp_log.h"
// #include "esp_nimble_hci.h"
// #include "nimble/nimble_port.h"
// #include "nimble/nimble_port_freertos.h"
// #include "host/ble_hs.h"
// #include "services/gap/ble_svc_gap.h"
// #include "services/gatt/ble_svc_gatt.h"
// #include "sdkconfig.h"

// static const char *TAG = "BLE-Server"; // Use static const for string literal
// uint8_t ble_addr_type;
// uint16_t connection_handle = 0;      // Store the connection handle
// uint16_t sensor_char_val_handle = 0; // Placeholder for sensor characteristic handle

// // Function prototypes
// void ble_app_advertise(void);
// static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
// static int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
// void notify_sensor_value(void);
// void notify_task(void *param);

// // Simulated sensor value generation
// int read_sensor_value()
// {
//     return rand() % 100; // Generate a random value between 0 and 99
// }

// // Write data to ESP32 defined as server
// static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
// {
//     char data[ctxt->om->om_len + 1];
//     memcpy(data, ctxt->om->om_data, ctxt->om->om_len);
//     data[ctxt->om->om_len] = '\0'; // Null-terminate the string

//     if (strncmp(data, "LED ON", ctxt->om->om_len) == 0)
//     {
//         gpio_set_level(GPIO_NUM_2, 1);
//         ESP_LOGI(TAG, "LED is ON");
//     }
//     else if (strncmp(data, "LED OFF", ctxt->om->om_len) == 0)
//     {
//         gpio_set_level(GPIO_NUM_2, 0);
//         ESP_LOGI(TAG, "LED is OFF");
//     }
//     else
//     {
//         ESP_LOGI(TAG, "Received unknown command: %s", data);
//     }
//     return 0;
// }

// static int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
// {
//     int sensor_value = read_sensor_value();
//     char sensor_str[16];
//     snprintf(sensor_str, sizeof(sensor_str), "Sensor: %d", sensor_value);
//     os_mbuf_append(ctxt->om, sensor_str, strlen(sensor_str)); // Send sensor value to client
//     return 0;
// }

// // Array of pointers to other service definitions
// static const struct ble_gatt_svc_def gatt_svcs[] = {
//     {.type = BLE_GATT_SVC_TYPE_PRIMARY,
//      .uuid = BLE_UUID16_DECLARE(0x1801), // GATT Service
//      .characteristics = (struct ble_gatt_chr_def[]){
//          {
//              .uuid = BLE_UUID16_DECLARE(0xFEF4), // Custom UUID for Sensor Data
//              .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
//              .access_cb = device_read,
//              .val_handle = &sensor_char_val_handle // Store the value handle
//          },
//          {.uuid = BLE_UUID16_DECLARE(0xDEAD), // Custom UUID for LED Control
//           .flags = BLE_GATT_CHR_F_WRITE,
//           .access_cb = device_write},
//          {0} // Terminate the array
//      }},
//     {0} // Terminate the array
// };

// static int ble_gap_event(struct ble_gap_event *event, void *arg)
// {
//     switch (event->type)
//     {
//     case BLE_GAP_EVENT_CONNECT:
//         ESP_LOGI(TAG, "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
//         if (event->connect.status == 0)
//         {
//             connection_handle = event->connect.conn_handle; // Store connection handle
//             ESP_LOGI("GAP", "Connected with handle: %d", connection_handle);
//         }
//         else
//         {
//             ESP_LOGE(TAG, "Connection failed, retrying advertisement.");
//             connection_handle = 0; // Reset the connection handle if failed
//             ble_app_advertise();   // Retry advertising if connection failed
//         }
//         break;

//     case BLE_GAP_EVENT_DISCONNECT:
//         ESP_LOGI(TAG, "BLE GAP EVENT DISCONNECTED");
//         connection_handle = 0; // Clear connection handle
//         ble_app_advertise();   // Restart advertising
//         break;

//     case BLE_GAP_EVENT_ADV_COMPLETE:
//         ESP_LOGI(TAG, "BLE GAP EVENT ADV COMPLETE");
//         ble_app_advertise(); // Restart advertising after advertising completes
//         break;

//     default:
//         ESP_LOGI(TAG, "Unhandled BLE GAP Event: %d", event->type); // Log any unhandled events
//         break;
//     }
//     return 0;
// }

// // Define the BLE connection
// void ble_app_advertise(void)
// {
//     struct ble_hs_adv_fields fields;
//     const char *device_name;
//     memset(&fields, 0, sizeof(fields));
//     device_name = ble_svc_gap_device_name(); // Retrieve device name
//     fields.name = (uint8_t *)device_name;
//     fields.name_len = strlen(device_name);
//     fields.name_is_complete = 1;
//     ble_gap_adv_set_fields(&fields); // Set advertising fields

//     struct ble_gap_adv_params adv_params;
//     memset(&adv_params, 0, sizeof(adv_params));
//     adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;                                             // Undirected connectable
//     adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;                                             // General discoverable
//     ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL); // Start advertising
// }

// void ble_app_on_sync(void)
// {
//     ble_hs_id_infer_auto(0, &ble_addr_type); // Automatically determine address type
//     ble_app_advertise();                     // Start advertising
// }

// void host_task(void *param)
// {
//     nimble_port_run(); // Runs the NimBLE host until stopped
// }

// void notify_sensor_value(void)
// {
//     if (connection_handle != 0)
//     {                                           // Ensure there's an active connection
//         int sensor_value = read_sensor_value(); // Simulate sensor reading
//         char sensor_str[16];
//         snprintf(sensor_str, sizeof(sensor_str), "Sensor: %d", sensor_value);

//         // Notify the client
//         int rc = ble_gatts_notify(connection_handle, sensor_char_val_handle);
//         if (rc != 0)
//         {
//             ESP_LOGE(TAG, "Notification failed with error: %d", rc);
//         }
//         else
//         {
//             ESP_LOGI("BLE", "Sending notification: %d", sensor_value);
//         }
//     }
//     else
//     {
//         ESP_LOGE("BLE", "No active connection to send notification");
//     }
// }

// void notify_task(void *param)
// {
//     while (1)
//     {
//         notify_sensor_value();           // Send sensor data notification
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 5 seconds
//     }
// }

// void app_main()
// {
//     // Initializ

//     // Initialize NVS
//     esp_err_t ret = nvs_flash_erase();
//     if (ret != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Failed to erase NVS partition: %s", esp_err_to_name(ret));
//     }
//     else
//     {
//         ESP_LOGI(TAG, "NVS partition erased successfully");
//     }

//     ret = nvs_flash_init(); // Initialize NVS flash
//     if (ret != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
//     }
//     else
//     {
//         ESP_LOGI(TAG, "NVS initialized successfully");
//     }

//     // Initialize NimBLE stack
//     nimble_port_init();
//     ble_svc_gap_device_name_set("BLE-Server"); // Set the BLE server name
//     ble_svc_gap_init();                        // Initialize the GAP service
//     ble_svc_gatt_init();                       // Initialize the GATT service
//     ble_gatts_count_cfg(gatt_svcs);            // Configure GATT services
//     ble_gatts_add_svcs(gatt_svcs);             // Add GATT services
//     ble_hs_cfg.sync_cb = ble_app_on_sync;      // Set sync callback
//     nimble_port_freertos_init(host_task);      // Start the NimBLE task

//     // Create a FreeRTOS task to periodically notify sensor values
//     xTaskCreate(notify_task, "notify_task", 4096, NULL, 5, NULL);
// }
#include <stdio.h>
#include <stdlib.h> // for random value generation
#include <string.h> // for strlen and snprintf
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sdkconfig.h"

static const char *TAG = "BLE-Server"; // Use static const for string literal
uint8_t ble_addr_type;
uint16_t connection_handle = 0;      // Store the connection handle
uint16_t sensor_char_val_handle = 0; // Placeholder for sensor characteristic handle
uint8_t count = 0;
// Function prototypes
void ble_app_advertise(void);
static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
void notify_sensor_value(void);
void notify_task(void *param);

// Simulated sensor value generation
int read_sensor_value()
{
    count++;
    return count; // Generate a random value between 0 and 99
}

// Write data to ESP32 defined as server
static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    char data[ctxt->om->om_len + 1];
    memcpy(data, ctxt->om->om_data, ctxt->om->om_len);
    data[ctxt->om->om_len] = '\0'; // Null-terminate the string
    return 0;
}

static int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int sensor_value = read_sensor_value();
    char sensor_str[16];
    snprintf(sensor_str, sizeof(sensor_str), "Sensor: %d", sensor_value);
    os_mbuf_append(ctxt->om, sensor_str, strlen(sensor_str)); // Send sensor value to client
    return 0;
}

// Array of pointers to other service definitions
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(0x1801), // GATT Service
     .characteristics = (struct ble_gatt_chr_def[]){
         {
             .uuid = BLE_UUID16_DECLARE(0xFEF4), // Custom UUID for Sensor Data
             .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
             .access_cb = device_read,
             .val_handle = &sensor_char_val_handle // Store the value handle
         },
         {.uuid = BLE_UUID16_DECLARE(0xDEAD), // Custom UUID for LED Control
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {0} // Terminate the array
     }},
    {0} // Terminate the array
};

static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status == 0)
        {
            connection_handle = event->connect.conn_handle; // Store connection handle
            ESP_LOGI("GAP", "Connected with handle: %d", connection_handle);
        }
        else
        {
            ESP_LOGE(TAG, "Connection failed, retrying advertisement.");
            connection_handle = 0; // Reset the connection handle if failed
            ble_app_advertise();   // Retry advertising if connection failed
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "BLE GAP EVENT DISCONNECTED");
        connection_handle = 0; // Clear connection handle
        ble_app_advertise();   // Restart advertising
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "BLE GAP EVENT ADV COMPLETE");
        ble_app_advertise(); // Restart advertising after advertising completes
        break;

    default:
        ESP_LOGI(TAG, "Unhandled BLE GAP Event: %d", event->type); // Log any unhandled events
        break;
    }
    return 0;
}

// Define the BLE connection
void ble_app_advertise(void)
{
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name(); // Retrieve device name
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields); // Set advertising fields

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;                                             // Undirected connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;                                             // General discoverable
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL); // Start advertising
}

void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type); // Automatically determine address type
    ble_app_advertise();                     // Start advertising
}

void host_task(void *param)
{
    nimble_port_run(); // Runs the NimBLE host until stopped
}

void notify_sensor_value(void)
{
    if (connection_handle != 0)
    {                                           // Ensure there's an active connection
        int sensor_value = read_sensor_value(); // Simulate sensor reading
        char sensor_str[16];
        snprintf(sensor_str, sizeof(sensor_str), "Sensor: %d", sensor_value);

        // Notify the client
        int rc = ble_gatts_notify(connection_handle, sensor_char_val_handle);
        if (rc != 0)
        {
            ESP_LOGE(TAG, "Notification failed with error: %d", rc);
        }
        else
        {
            ESP_LOGI("BLE", "Sending notification: %d", sensor_value);
        }
    }
}

void notify_task(void *param)
{
    while (1)
    {
        notify_sensor_value();           // Send sensor data notification
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 5 seconds
    }
}

void app_main()
{

    nvs_flash_erase();
    esp_err_t ret = nvs_flash_init(); // Initialize NVS flash
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "NVS initialized successfully");
    }
    nimble_port_init();
    ble_svc_gap_device_name_set("BLE-Server"); // Set the BLE server name
    ble_svc_gap_init();                        // Initialize the GAP service
    ble_svc_gatt_init();                       // Initialize the GATT service
    ble_gatts_count_cfg(gatt_svcs);            // Configure GATT services
    ble_gatts_add_svcs(gatt_svcs);             // Add GATT services
    ble_hs_cfg.sync_cb = ble_app_on_sync;      // Set sync callback
    nimble_port_freertos_init(host_task);      // Start the NimBLE task

    xTaskCreate(notify_task, "notify_task", 4096, NULL, 5, NULL);
}
