#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define BLINK_GPIO GPIO_NUM_4
#define CHECK_GPIO GPIO_NUM_2
static const char *TAG = "LED_BLINK";

void app_main(void)
{
    gpio_config_t io_conf;

    // Configure BLINK_GPIO as an output
    io_conf.intr_type = GPIO_INTR_DISABLE;    // No interrupt
    io_conf.mode = GPIO_MODE_OUTPUT;          // Set as output
    io_conf.pin_bit_mask = (1ULL << BLINK_GPIO); // Bitmask of the GPIOs to configure
    io_conf.pull_down_en = 0;                 // Disable pull-down resistor
    io_conf.pull_up_en = 0;                   // Disable pull-up resistor
    gpio_config(&io_conf);

    gpio_config_t io_conf_check;

    // Configure CHECK_GPIO as an input
    io_conf_check.intr_type = GPIO_INTR_DISABLE;    // No interrupt
    io_conf_check.mode = GPIO_MODE_INPUT;          // Set as input
    io_conf_check.pin_bit_mask = (1ULL << CHECK_GPIO); // Bitmask of the GPIOs to configure
    io_conf_check.pull_down_en = 0;                 // Disable pull-down resistor
    io_conf_check.pull_up_en = 0;                   // Disable pull-up resistor
    gpio_config(&io_conf_check);

    while (1)
    {
        gpio_set_level(BLINK_GPIO, 0);  // Turn LED OFF
        ESP_LOGI(TAG, "LED OFF, GPIO Level: %d", gpio_get_level(CHECK_GPIO));
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        gpio_set_level(BLINK_GPIO, 1);  // Turn LED ON
        ESP_LOGI(TAG, "LED ON, GPIO Level: %d", gpio_get_level(CHECK_GPIO));
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
