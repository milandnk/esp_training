#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_intr_alloc.h"
#include "soc/uart_periph.h"

static const char *TAG = "uart_events";
#define EX_UART_NUM UART_NUM_0
#define BUF_SIZE (1024)

static intr_handle_t uart_isr_handle;

/* ISR handler function */
static void IRAM_ATTR uart_isr_handler(void *arg)
{
    uint8_t data[BUF_SIZE];
    int length = uart_read_bytes(EX_UART_NUM, data, BUF_SIZE, 10 / portTICK_PERIOD_MS);
    if (length > 0)
    {
        data[length] = '\0';                      // Null-terminate for printing
        ESP_LOGI(TAG, "Data received: %s", data); // Log received data
    }
}

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    /* Configure UART parameters */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Install UART driver
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(EX_UART_NUM, &uart_config);

    // Enable RX interrupt
   
    // Allocate the interrupt handler
    esp_err_t err = esp_intr_alloc(uart_periph_signal[EX_UART_NUM].irq, ESP_INTR_FLAG_IRAM, uart_isr_handler, NULL, &uart_isr_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to allocate interrupt: %s", esp_err_to_name(err));
        return;
    }

    while (1)
    {
        ESP_LOGI(TAG, "Waiting for data...");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}


