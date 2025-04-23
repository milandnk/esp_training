// #include <stdint.h>
// #include <string.h>
// #include "esp_log.h"               // For logging messages
// #include "freertos/FreeRTOS.h"     // FreeRTOS headers for tasks and queues
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "tinyusb.h"               // TinyUSB headers for USB device handling
// #include "tusb_cdc_acm.h"
// #include "sdkconfig.h"             // ESP-IDF configuration
// #include "driver/uart.h"           // UART driver for serial communication

// // Log tag for identifying messages in logs
// static const char *TAG = "USB_CDC_Uart";

// // Buffer for receiving data from USB
// static uint8_t rx_buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];

// // Queues for UART and app messages
// static QueueHandle_t uart0_queue;
// static QueueHandle_t app_queue;

// // Buffer size for UART reads
// #define RD_BUF_SIZE (1024)

// // Structure for holding messages sent to the app queue
// typedef struct {
//     uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1]; // Data buffer
//     size_t buf_len;                                 // Length of data in the buffer
//     uint8_t itf;                                    // Interface number
// } app_message_t;

// // UART event handling task
// void uart_event_task(void *pvParameters) {
//     uart_event_t event;
//     uint8_t *dtmp = (uint8_t *) malloc(RD_BUF_SIZE); // Temporary buffer for reading UART data

//     for (;;) {
//         // Wait for UART events from the UART queue
//         if (xQueueReceive(uart0_queue, (void *)&event, portMAX_DELAY)) {
//             bzero(dtmp, RD_BUF_SIZE); // Clear temporary buffer

//             switch (event.type) {
//                 case UART_DATA:
//                     // If data is available, read it into `dtmp` and print it in hexadecimal format
//                     uart_read_bytes(UART_NUM_0, dtmp, event.size, portMAX_DELAY);
//                     ESP_LOGI(TAG, "UART DATA:");
//                     ESP_LOG_BUFFER_HEXDUMP(TAG, dtmp, event.size, ESP_LOG_INFO);

//                     // Send UART data to USB CDC interface
//                     tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, dtmp, event.size);
//                     tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, 0);
//                     break;
//                 default:
//                     break;
//             }
//         }
//     }
//     free(dtmp); // Free allocated buffer (not reachable due to infinite loop)
//     vTaskDelete(NULL); // Delete task (not reachable)
// }

// // Callback function when USB CDC receives data
// void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event) {
//     size_t rx_size = 0;
//     // Read data from USB CDC into `rx_buf`
//     esp_err_t ret = tinyusb_cdcacm_read(itf, rx_buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);

//     if (ret == ESP_OK) {
//         // Create a message for the app queue
//         app_message_t tx_msg = {
//             .buf_len = rx_size,
//             .itf = itf,
//         };
//         memcpy(tx_msg.buf, rx_buf, rx_size);
//         xQueueSend(app_queue, &tx_msg, 0); // Send the message to the app queue
//     } else {
//         ESP_LOGE(TAG, "Read Error"); // Log an error if data couldn't be read
//     }
// }

// // Callback function when USB CDC line state changes (DTR/RTS signals)
// void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event) {
//     int dtr = event->line_state_changed_data.dtr; // Data Terminal Ready
//     int rts = event->line_state_changed_data.rts; // Request to Send
//     ESP_LOGI(TAG, "Line state changed on channel %d: DTR:%d, RTS:%d", itf, dtr, rts);
// }

// // Main application entry point
// void app_main(void) {
//     // Create FreeRTOS queues
//     app_queue = xQueueCreate(5, sizeof(app_message_t)); // Queue for app messages
//     assert(app_queue);
//     uart0_queue = xQueueCreate(20, sizeof(uart_event_t)); // Queue for UART events
//     assert(uart0_queue);
//     app_message_t msg;

//     ESP_LOGI(TAG, "USB initialization");

//     // TinyUSB configuration
//     const tinyusb_config_t tusb_cfg = {
//         .device_descriptor = NULL,
//         .string_descriptor = NULL,
//         .external_phy = false,
// #if (TUD_OPT_HIGH_SPEED)
//         .fs_configuration_descriptor = NULL,
//         .hs_configuration_descriptor = NULL,
//         .qualifier_descriptor = NULL,
// #else
//         .configuration_descriptor = NULL,
// #endif
//     };

//     // Install the TinyUSB driver
//     ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

//     // CDC ACM (serial over USB) configuration
//     tinyusb_config_cdcacm_t acm_cfg = {
//         .usb_dev = TINYUSB_USBDEV_0,
//         .cdc_port = TINYUSB_CDC_ACM_0,
//         .rx_unread_buf_sz = 64,
//         .callback_rx = &tinyusb_cdc_rx_callback,                  // Set callback for receiving data
//         .callback_rx_wanted_char = NULL,
//         .callback_line_state_changed = &tinyusb_cdc_line_state_changed_callback, // Set callback for line state changes
//         .callback_line_coding_changed = NULL
//     };

//     // Initialize TinyUSB CDC ACM
//     ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));

//     ESP_LOGI(TAG, "USB initialization DONE");

//     // UART Configuration
//     const uart_config_t uart_config = {
//         .baud_rate = 115200,               // Baud rate for UART
//         .data_bits = UART_DATA_8_BITS,     // Data bits (8)
//         .parity = UART_PARITY_DISABLE,     // Parity (none)
//         .stop_bits = UART_STOP_BITS_1,     // Stop bits (1)
//         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, // Flow control (none)
//     };
//     ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
//     ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
//     ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, RD_BUF_SIZE * 2, 0, 20, &uart0_queue, 0));

//     // Create UART event handling task
//     xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);

//     // Main loop to receive messages from the app queue
//     while (1) {
//         if (xQueueReceive(app_queue, &msg, portMAX_DELAY)) {
//             if (msg.buf_len) {
//                 // Print received data in hexadecimal format
//                 ESP_LOGI(TAG, "Data from channel %d:", msg.itf);
//                 ESP_LOG_BUFFER_HEXDUMP(TAG, msg.buf, msg.buf_len, ESP_LOG_INFO);
//             }
//         }
//     }
// }

#include <stdint.h>
#include <string.h>
#include "esp_log.h"           // For logging messages
#include "freertos/FreeRTOS.h" // FreeRTOS headers for tasks and queues
#include "freertos/task.h"
#include "freertos/queue.h"
#include "tinyusb.h" // TinyUSB headers for USB device handling
#include "tusb_cdc_acm.h"
#include "sdkconfig.h" // ESP-IDF configuration

// Log tag for identifying messages in logs
static const char *TAG = "USB_CDC_Echo";

// Buffer for receiving data from USB
static uint8_t rx_buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];

// Queue for application messages
static QueueHandle_t app_queue;

// Structure for holding messages sent to the app queue
typedef struct
{
    uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1]; // Data buffer
    size_t buf_len;                                 // Length of data in the buffer
    uint8_t itf;                                    // Interface number
} app_message_t;

// Callback function when USB CDC receives data
void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    size_t rx_size = 0;
    // Read data from USB CDC into `rx_buf`
    esp_err_t ret = tinyusb_cdcacm_read(itf, rx_buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);

    if (ret == ESP_OK)
    {
        // Create a message for the app queue
        app_message_t tx_msg = {
            .buf_len = rx_size,
            .itf = itf,
        };
        memcpy(tx_msg.buf, rx_buf, rx_size);
        xQueueSend(app_queue, &tx_msg, 0); // Send the message to the app queue
    }
    else
    {
        ESP_LOGE(TAG, "Read Error"); // Log an error if data couldn't be read
    }
}

// Callback function when USB CDC line state changes (DTR/RTS signals)
void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event)
{
    int dtr = event->line_state_changed_data.dtr; // Data Terminal Ready
    int rts = event->line_state_changed_data.rts; // Request to Send
    ESP_LOGI(TAG, "Line state changed on channel %d: DTR:%d, RTS:%d", itf, dtr, rts);
}

// Main application entry point
void app_main(void)
{
    // Create FreeRTOS queue for app messages
    app_queue = xQueueCreate(5, sizeof(app_message_t)); // Queue for app messages
    assert(app_queue);
    app_message_t msg;

    ESP_LOGI(TAG, "USB initialization");

    // TinyUSB configuration
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
#if (TUD_OPT_HIGH_SPEED)
        .fs_configuration_descriptor = NULL,
        .hs_configuration_descriptor = NULL,
        .qualifier_descriptor = NULL,
#else
        .configuration_descriptor = NULL,
#endif
    };

    // Install the TinyUSB driver
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    // CDC ACM (serial over USB) configuration
    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = &tinyusb_cdc_rx_callback, // Set callback for receiving data
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = &tinyusb_cdc_line_state_changed_callback, // Set callback for line state changes
        .callback_line_coding_changed = NULL};

    // Initialize TinyUSB CDC ACM
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));

    ESP_LOGI(TAG, "USB initialization DONE");

    // Main loop to receive and echo messages from the app queue
    while (1)
    {
        if (xQueueReceive(app_queue, &msg, portMAX_DELAY))
        {
            if (msg.buf_len)
            {
                tinyusb_cdcacm_write_queue(msg.itf, msg.buf, msg.buf_len);
                tinyusb_cdcacm_write_flush(msg.itf, 0);
            }
        }
    }
}
