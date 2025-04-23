#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_GPIO GPIO_NUM_38

static const char *TAG = "uart_led_control";

static void uart_led_control_task(void *arg)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    const int uart_buffer_size = (1024 * 2);
    uart_driver_install(UART_NUM_0, uart_buffer_size, uart_buffer_size, 0, NULL, 0);

    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    uint8_t data[2];

    while (1)
    {
        ESP_LOGI(TAG, "waiting for input ??");
        int len = uart_read_bytes(UART_NUM_0, data, 1, 1000 / portTICK_PERIOD_MS);

        if (len > 0)
        {
            data[len] = '\0';
            ESP_LOGI(TAG, "Received: %s", data);

            if (data[0] == '1')
            {
                
                gpio_set_level(LED_GPIO, 1);
                ESP_LOGI(TAG, "LED ON");
            }
            else if (data[0] == '0')
            {
                gpio_set_level(LED_GPIO, 0);
                ESP_LOGI(TAG, "LED OFF");
            }
            else
            {
                ESP_LOGI(TAG, "Invalid character");
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

void app_main(void)
{
    xTaskCreate(uart_led_control_task, "uart_led_control_task", 4096, NULL, 10, NULL);
}