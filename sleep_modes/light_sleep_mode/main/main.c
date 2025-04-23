// #include <stdio.h>
// #include <time.h>
// #include <sys/time.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_sleep.h"
// #include "esp_log.h"
// #include "driver/gpio.h"

// #define LED_PIN GPIO_NUM_2 // Use the appropriate GPIO pin for your LED
// static const char *TAG = "light_sleep_example";

// void toggle_led()
// {
//     gpio_set_level(LED_PIN, 1);

//     vTaskDelay(pdMS_TO_TICKS(5000)); // Delay for 1 second (1000 ms)
// }

// void app_main(void)
// {
//     gpio_config_t io_conf;
//     io_conf.intr_type = GPIO_INTR_DISABLE;
//     io_conf.mode = GPIO_MODE_OUTPUT;
//     io_conf.pin_bit_mask = (1ULL << LED_PIN);
//     io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
//     io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
//     gpio_config(&io_conf);

//     ESP_LOGI(TAG, "Configuring timer to wake up every 5 seconds...");

//     int wakeup_count = 0;

//     while (1)
//     {
//         ESP_LOGI(TAG, "Entering light sleep mode...");
//         // Set wakeup timer
//         esp_sleep_enable_timer_wakeup(10000000); // Wake up in 5 seconds (5 million microseconds)

//         // Enter light sleep mode
//         esp_light_sleep_start();

//         // Code to run after waking up
//         ESP_LOGI(TAG, "Woke up from light sleep!");

//         wakeup_count++;
//         ESP_LOGI(TAG, "wake up count %d", wakeup_count);
//         toggle_led();
//     }
// }

// #include <stdio.h>
// #include "esp_sleep.h"
// #include "esp_log.h"
// #include "driver/gpio.h"
// #include "driver/i2c.h"
// #include "driver/spi_common.h"
// #include "driver/uart.h"
// #include "esp_wifi.h"
// #include "esp_bt.h"

// #define WAKEUP_TIME_SEC 300 // Wake up every 5 minutes (300 seconds)

// static const char *TAG = "DEEP_SLEEP";

// void disable_all_peripherals(void)
// {
//     // Disable Wi-Fi and Bluetooth
//     esp_wifi_stop();
//     esp_bt_controller_disable();

//     // Disable I2C
//     i2c_driver_delete(I2C_NUM_0);

//     // Disable SPI
//     spi_bus_free(SPI2_HOST);

//     // Disable UART
//     uart_driver_delete(UART_NUM_0);
//     uart_driver_delete(UART_NUM_1);
// }

// void app_main(void)
// {
//     // Log message indicating start of wake-up event handling
//     ESP_LOGI(TAG, "Wakeup event detected, starting main application...");

//     // Disable all peripherals to save power
//     disable_all_peripherals();

//     // Power down all digital IO and keep only RTC IO for deep sleep
//     // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);  // Keep RTC periph on for wakeup
//     // esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);       // Turn off crystal oscillator

//     // Log message to indicate entering deep sleep
//     ESP_LOGI(TAG, "Entering deep sleep for %d seconds...", WAKEUP_TIME_SEC);

//     // Configure timer to wake up every 5 minutes
//     esp_sleep_enable_timer_wakeup(WAKEUP_TIME_SEC * 1000000); // Convert seconds to microseconds

//     // Enter deep sleep mode
//     esp_deep_sleep_start();
// }
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "esp_adc/adc_oneshot.h" // New ADC oneshot driver header

// Function to initialize and erase NVS
void erase_nvs_flash()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_OK)
    {
        ret = nvs_flash_erase();
        if (ret == ESP_OK)
        {
            printf("NVS flash erased successfully.\n");
        }
        else
        {
            printf("Failed to erase NVS flash. Error: %s\n", esp_err_to_name(ret));
        }
    }
    else
    {
        printf("Failed to initialize NVS. Error: %s\n", esp_err_to_name(ret));
    }
}

void deactivate_peripherals()
{
    // Deinitialize I2C
    i2c_driver_delete(I2C_NUM_0);
    // Deinitialize SPI
    spi_bus_free(SPI2_HOST);

    // Deinitialize UART
    uart_driver_delete(UART_NUM_0);
    uart_driver_delete(UART_NUM_1);

    // New ADC API does not require explicit power release
    // Any initialized ADC driver instances should be deleted here if needed.
}

void setup_gpio_low_power()
{
    for (int i = 0; i < GPIO_NUM_MAX; i++)
    {
        gpio_reset_pin((gpio_num_t)i);
        gpio_set_direction((gpio_num_t)i, GPIO_MODE_INPUT);
        gpio_set_pull_mode((gpio_num_t)i, GPIO_FLOATING);

        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_DISABLE;
        io_conf.pin_bit_mask = (gpio_num_t)i;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&io_conf));
    }
}

void enter_deep_sleep()
{
    setup_gpio_low_power();

    printf("Entering deep sleep mode...\n");
    esp_deep_sleep_start();
}

void app_main()
{
    // Step 1: Erase NVS to clear stored data
    erase_nvs_flash();

    // Step 2: Deactivate peripherals
    deactivate_peripherals();

    // Optional message
    printf("All peripherals deactivated, NVS and GPIO reset. Preparing for deep sleep.\n");

    // Step 3: Enter deep sleep
    enter_deep_sleep();
}
