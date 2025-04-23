#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc_cal.h"
#include "esp_timer.h"

#define TAG "ADC_TIMER_ISR"
#define ADC_CHANNEL ADC1_CHANNEL_0  // GPIO1
#define ADC_ATTEN ADC_ATTEN_DB_11   // 11dB attenuation for 3.3V range
#define READ_LEN 1024               // Buffer length for ADC

static adc_continuous_handle_t adc_handle;
esp_timer_handle_t periodic_timer;  // Timer handle

static void read_adc_data(void)
{
    uint8_t adc_data[READ_LEN] = {0};
    uint32_t out_length = 0;
    int raw_adc;

    // Read ADC data using the continuous driver
    esp_err_t ret = adc_continuous_read(adc_handle, adc_data, READ_LEN, &out_length, 0);
    if (ret == ESP_OK && out_length > 0)
    {
        for (int i = 0; i < out_length; i += sizeof(adc_digi_output_data_t))
        {
            adc_digi_output_data_t *adc_output = (adc_digi_output_data_t *)&adc_data[i];
            raw_adc = adc_output->type2.data;
            ESP_LOGI(TAG, "ADC: %d, Voltage: %.2fV", raw_adc, ((raw_adc * 3.3) / 4095));
        }
    }
}

static void periodic_timer_callback(void *arg)
{
    ESP_LOGI(TAG, "Timer triggered, reading ADC");
    read_adc_data();  
}

void init_adc_dma(void)
{
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,    // Buffer size for DMA
        .conv_frame_size = READ_LEN,   // Size of each ADC frame to read
    };

    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_handle));

    adc_continuous_config_t adc_dma_config = {
        .sample_freq_hz = 20 * 1000,   // 20 kHz sample rate
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,  // Single ADC unit (ADC1)
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,  // 12-bit output format
    };

    adc_digi_pattern_config_t adc_pattern = {
        .atten = ADC_ATTEN_DB_11,      // 11 dB attenuation for 0-3.3V input
        .channel = ADC_CHANNEL,
        .bit_width = ADC_WIDTH_BIT_12  // 12-bit resolution
    };
    adc_dma_config.adc_pattern = &adc_pattern;
    adc_dma_config.pattern_num = 1;

    ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &adc_dma_config));
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));
}

void init_timer(void)
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,  // Timer callback
        .name = "periodic_timer"               // Timer name
    };
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 5000000));
}

void app_main(void)
{
    init_adc_dma();

    init_timer();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));  
    }
}
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/adc.h"
// #include "esp_adc/adc_continuous.h"
// #include "esp_log.h"

// #define ADC_CHANNEL ADC1_CHANNEL_6   // ADC1 Channel 6 corresponds to GPIO34
// #define SAMPLE_FREQ_HZ 1000          // Sampling frequency

// static const char *TAG = "POT_ADC_INTERRUPT";

// #define BUFFER_SIZE 1024

// // Callback function when ADC buffer is full
// static void adc_continuous_handler(void *arg)
// {
//     uint8_t buffer[BUFFER_SIZE];
//     uint32_t bytes_read = 0;

//     esp_err_t ret = adc_continuous_read((adc_continuous_handle_t)arg, buffer, BUFFER_SIZE, &bytes_read, portMAX_DELAY);
//     if (ret == ESP_OK) {
//         for (int i = 0; i < bytes_read; i += 4) {  // Assuming 12-bit data
//             uint32_t adc_value = ((uint32_t)buffer[i + 1] << 8) | buffer[i];
//             ESP_LOGI(TAG, "Potentiometer Value: %ld", adc_value);
//         }
//     } else {
//         ESP_LOGE(TAG, "Failed to read ADC data");
//     }
// }

// void app_main(void)
// {
//     // ADC continuous configuration
//     adc_continuous_handle_t handle;

//     adc_continuous_config_t adc_config = {
//         .pattern_num = 1,
//         .sample_freq_hz = SAMPLE_FREQ_HZ,         // Sampling frequency
//         .conv_mode = ADC_CONV_SINGLE_UNIT_1,
//         .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1    // Output format
//     };

//     // Initialize ADC pattern configuration
//     adc_digi_pattern_config_t adc_pattern[1] = {
//         {
//             .atten = ADC_ATTEN_DB_0,            // No attenuation (input range: 0-1.1V)
//             .bit_width = ADC_BITWIDTH_DEFAULT,  // Default bit width (usually 12 bits)
//             .channel = ADC_CHANNEL,             // ADC1 Channel 6 (GPIO34)
//             .unit = ADC_UNIT_1
//         }
//     };

//     // Attach pattern to configuration
//     adc_config.adc_pattern = adc_pattern;
//     adc_config.pattern_num = sizeof(adc_pattern) / sizeof(adc_pattern[0]);

//     // Initialize ADC continuous driver
//     ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

//     // Start ADC continuous conversion
//     ESP_ERROR_CHECK(adc_continuous_start(handle));

//     // Create a task to handle ADC data
//     xTaskCreate(adc_continuous_handler, "adc_handler", 4096, (void*)handle, 5, NULL);

//     // Main loop
//     while (1) {
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Keep main task running
//     }
// }