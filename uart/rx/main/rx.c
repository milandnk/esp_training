// #include <stdio.h>
// #include <string.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/uart.h"
// #include "driver/gpio.h"
// #include "esp_log.h"

// static const char *TAG = "uart_test";

// static void uart_task(void *arg)
// {
//     static const uart_config_t uart_config = {
//         .baud_rate = 115200,
//         .data_bits = UART_DATA_8_BITS,
//         .parity = UART_PARITY_DISABLE,
//         .stop_bits = UART_STOP_BITS_1,
//         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//         .source_clk = UART_SCLK_DEFAULT};

//     ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));

//     ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, GPIO_NUM_1, GPIO_NUM_3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

//     const int uart_buffer_size = (1024 * 2);
//     QueueHandle_t uart_queue;

//     ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));

//     uint8_t data[128];

//     uart_flush_input(UART_NUM_0);

//     while (1)
//     {
//         ESP_LOGI(TAG, "Waiting to receive.....");

//         int len = uart_read_bytes(UART_NUM_0, data, sizeof(data) - 1, 100);

//         if (len >= 0)
//         {
//             data[len] = '\0';

//             ESP_LOGI(TAG, "Received data: %s", (char *)data);

//             uart_write_bytes(UART_NUM_0, (const char *)data, len);
//             ESP_LOGI(TAG, "Sent data back: %s", (char *)data);
//         }
//         else if(len < 0)
//         {
//            ESP_LOGI(TAG, "No received!!");
//         }

//         vTaskDelay(500 / portTICK_PERIOD_MS);
//     }
// }

// void app_main(void)
// {

//     xTaskCreate(uart_task, "uart_task", 4096, NULL, 10, NULL);
// }
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "uart_test";

static void uart_task(void *arg)
{
    static const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT};

    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    const int uart_buffer_size = (1024 * 2);
    QueueHandle_t uart_queue;

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));

    uint8_t data[128];

    uart_flush_input(UART_NUM_0);

    while (1)
    {
        ESP_LOGI(TAG, "Waiting to receive.....");

        int len = uart_read_bytes(UART_NUM_0, data, sizeof(data) - 1, pdMS_TO_TICKS(10000)); // Increased timeout to 500 ms

        if (len >= 0)
        {
            data[len] = '\0';

            ESP_LOGI(TAG, "Received data: %s", (char *)data);

            uart_write_bytes(UART_NUM_0, (const char *)data, len);
            ESP_LOGI(TAG, "Sent data back: %s", (char *)data);
        }
        else if (len < 0)
        {
            ESP_LOGI(TAG, "No received!!");
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    xTaskCreate(uart_task, "uart_task", 4096, NULL, 10, NULL);
}
