// #include <stdio.h>
// #include <string.h>
// #include "driver/spi_master.h"
// #include "driver/gpio.h"
// #include "esp_log.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/timers.h"

// #define SPI_MOSI_PIN 12
// #define SPI_MISO_PIN 13
// #define SPI_SCLK_PIN 15
// #define SPI_CS_PIN 14

// static const char *TAG = "SPI_DMA";

// spi_device_handle_t spi;

// char rec_buf[10] = {0};

// volatile bool data_received = false;
// char send_buf[10] = "milan";

// void IRAM_ATTR gpio_isr_handler(void *arg)
// {
//     data_received = true;
// }

// void gpio_init()
// {
//     gpio_config_t io_conf = {
//         .intr_type = GPIO_INTR_ANYEDGE,
//         .mode = GPIO_MODE_INPUT,
//         .pin_bit_mask = (1ULL << SPI_MISO_PIN),
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .pull_up_en = GPIO_PULLUP_ENABLE,
//     };
//     gpio_config(&io_conf);

//     gpio_install_isr_service(0);
//     gpio_isr_handler_add(SPI_MISO_PIN, gpio_isr_handler, (void *)SPI_MISO_PIN);
// }

// void spi_master_init(void)
// {
//     spi_bus_config_t buscfg = {
//         .mosi_io_num = SPI_MOSI_PIN,
//         .miso_io_num = SPI_MISO_PIN,
//         .sclk_io_num = SPI_SCLK_PIN,
//         .quadwp_io_num = -1,
//         .quadhd_io_num = -1,
//         .max_transfer_sz = 4096,
//     };

//     spi_device_interface_config_t devcfg = {
//         .clock_speed_hz = 1000000,
//         .mode = 0,
//         .spics_io_num = SPI_CS_PIN,
//         .queue_size = 1,
//     };

//     esp_err_t ret = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
//     ESP_ERROR_CHECK(ret);

//     ret = spi_bus_add_device(SPI3_HOST, &devcfg, &spi);
//     ESP_ERROR_CHECK(ret);
// }

// void spi_send(char *send_data, size_t length)
// {
//     spi_transaction_t t;
//     memset(&t, 0, sizeof(t));

//     t.length = length * 8;
//     t.tx_buffer = send_data;

//     esp_err_t ret = spi_device_transmit(spi, &t);
//     ESP_ERROR_CHECK(ret);
// }

// void timer_callback(TimerHandle_t xTimer)
// {

//     spi_send(send_buf, strlen(send_buf) + 1);
// }

// void app_main(void)
// {
//     spi_master_init();
//     gpio_init();

//     TimerHandle_t timer = xTimerCreate("DataTxTimer", pdMS_TO_TICKS(5000), pdTRUE, (void *)0, timer_callback);
//     if (timer == NULL)
//     {
//         ESP_LOGE(TAG, "Failed to create timer");
//         return;
//     }

//     if (xTimerStart(timer, 0) != pdPASS)
//     {
//         ESP_LOGE(TAG, "Failed to start timer");
//         return;
//     }

//     while (1)
//     {
//         if (data_received)
//         {
//             ESP_LOGI(TAG, "data receive");
//             ESP_LOGI(TAG, "Sent data: %s", send_buf);
//             data_received = false;
//         }
//         ESP_LOGI(TAG, "hii");
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
#include <stdio.h>
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h" // Include semaphore header

#define SPI_MOSI_PIN 12
#define SPI_MISO_PIN 13
#define SPI_SCLK_PIN 15
#define SPI_CS_PIN 14

static const char *TAG = "SPI_DMA";

spi_device_handle_t spi;

char rec_buf[10] = {0};
char send_buf[10] = "milan";

// Declare a semaphore handle
SemaphoreHandle_t xSemaphore = NULL;

// ISR for MISO pin
void IRAM_ATTR gpio_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

void gpio_init()
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << SPI_MISO_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(SPI_MISO_PIN, gpio_isr_handler, (void *)SPI_MISO_PIN);
}

void spi_master_init(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = SPI_MOSI_PIN,
        .miso_io_num = SPI_MISO_PIN,
        .sclk_io_num = SPI_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1000000,
        .mode = 0,
        .spics_io_num = SPI_CS_PIN,
        .queue_size = 1,
    };

    esp_err_t ret = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    ret = spi_bus_add_device(SPI3_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}

void spi_send(char *send_data, size_t length)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    t.length = length * 8;
    t.tx_buffer = send_data;

    esp_err_t ret = spi_device_transmit(spi, &t);
    ESP_ERROR_CHECK(ret);
}

void timer_callback(TimerHandle_t xTimer)
{
    spi_send(send_buf, strlen(send_buf) + 1);
}

void data_receive_task(void *pvParameters)
{
    while (1)
    {
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "interrupt triggered.");
            ESP_LOGI(TAG, "Sent data: %s", send_buf);
        }
    }
}

void app_main(void)
{
    spi_master_init();
    gpio_init();

    xSemaphore = xSemaphoreCreateBinary();
    if (xSemaphore == NULL)
    {
        ESP_LOGE(TAG, "Failed to create semaphore");
        return;
    }
    xTaskCreate(data_receive_task, "DataReceiveTask", 2048, NULL, 10, NULL);
    TimerHandle_t timer = xTimerCreate("DataTxTimer", pdMS_TO_TICKS(5000), pdTRUE, (void *)0, timer_callback);
    if (timer == NULL)
    {
        ESP_LOGE(TAG, "Failed to create timer");
        return;
    }

    if (xTimerStart(timer, 0) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to start timer");
        return;
    }
    while (1)
    {
        ESP_LOGE(TAG, "hii");
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
}
