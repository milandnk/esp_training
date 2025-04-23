#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/timer.h"
#include "driver/gpio.h"

// Define the GPIO for the boot button
#define BOOT_BUTTON_GPIO GPIO_NUM_0

static const char *TAG = "InputCapture";
static uint64_t last_capture_time = 0; // Store the last captured time
static uint64_t time_diff = 0;         // Time difference between edges
#define TIMER_GROUP TIMER_GROUP_0      // Use timer group 0
#define TIMER_ID TIMER_0               // Use timer 0
volatile bool flag = false;            // Flag to indicate an event

void IRAM_ATTR boot_button_isr(void *arg)
{
    uint64_t current_time;
    timer_get_counter_value(TIMER_GROUP, TIMER_ID, &current_time); // Get current timer value

    if (last_capture_time != 0)
    {
        time_diff = current_time - last_capture_time; // Calculate time difference
        flag = true;                                  // Set flag to indicate event
    }

    last_capture_time = current_time; // Update last capture time
}

void app_main(void)
{
    ESP_LOGI(TAG, "Application started");

    // Configure the boot button GPIO
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE; // Trigger on falling edge
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BOOT_BUTTON_GPIO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // Disable pull-down
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;      // Enable pull-up
    gpio_config(&io_conf);

    // Configure the timer
    timer_config_t timer_config = {
        .divider = 80, // 1 MHz (80 MHz APB clock divided by 80)
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_START,
        .alarm_en = TIMER_ALARM_DIS,
        .auto_reload = TIMER_AUTORELOAD_DIS,
    };

    ESP_ERROR_CHECK(timer_init(TIMER_GROUP, TIMER_ID, &timer_config));  // Initialize timer
    ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP, TIMER_ID, 0)); // Reset timer counter
    ESP_ERROR_CHECK(timer_start(TIMER_GROUP, TIMER_ID));                // Start the timer

    // Register ISR for the boot button
    ESP_ERROR_CHECK(gpio_install_isr_service(0));                                   // Install GPIO ISR service
    ESP_ERROR_CHECK(gpio_isr_handler_add(BOOT_BUTTON_GPIO, boot_button_isr, NULL)); // Add ISR handler

    ESP_LOGI(TAG, "GPIO and Timer configured, waiting for button press...");

    // Main loop
    while (1)
    {
        if (flag)
        {
            flag = false;
            ESP_LOGI(TAG, "Time difference: %llu microseconds", time_diff);
            double time_diff_sec = (double)time_diff / 1000000.0; // Convert to seconds
            if (time_diff_sec > 0)
            {                                                   // Avoid division by zero
                double frequency = 1.0 / time_diff_sec;         // Frequency = 1 / period
                ESP_LOGI(TAG, "Frequency: %.4f Hz", frequency); // Log the calculated frequency
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay for a short time
    }
}
