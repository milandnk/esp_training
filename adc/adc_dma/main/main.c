#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc_cal.h"

#define TAG "ADC_DMA"
#define ADC_CHANNEL ADC1_CHANNEL_0 // GPIO1
#define ADC_ATTEN ADC_ATTEN_DB_11  // 11dB attenuation for 3.3V range
#define READ_LEN 1024              

static adc_continuous_handle_t adc_handle;

void init_adc_dma(void)
{
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,     // Buffer size for DMA
        .conv_frame_size = READ_LEN,    // Size of each ADC frame to read
    };

    adc_continuous_new_handle(&adc_config, &adc_handle);

    adc_continuous_config_t adc_dma_config = {
        .sample_freq_hz = 20 * 1000, // 20 kHz sample rate
        .conv_mode = ADC_CONV_SINGLE_UNIT_1, // Single ADC unit (ADC1)
        .format =  ADC_DIGI_OUTPUT_FORMAT_TYPE2, // 12-bit output format
    };

    adc_digi_pattern_config_t adc_pattern = {
        .atten = ADC_ATTEN_DB_11, // 11 dB attenuation for 0-3.3V input
        .channel = ADC_CHANNEL,   
        .bit_width = ADC_WIDTH_BIT_12 // 12-bit resolution
    };
    adc_dma_config.adc_pattern = &adc_pattern;
    adc_dma_config.pattern_num = 1;

    // Apply configuration and start continuous ADC
    adc_continuous_config(adc_handle, &adc_dma_config);
    adc_continuous_start(adc_handle);
}

void app_main(void)
{
    init_adc_dma(); 

    uint8_t adc_data[READ_LEN] = {0};  
    uint32_t out_length = 0; 
    int raw_adc;
    
    while (1)
    {
        esp_err_t ret = adc_continuous_read(adc_handle, adc_data, READ_LEN, &out_length, portMAX_DELAY);
        if (ret == ESP_OK )
        {
            for (int i = 0; i < 4; i += sizeof(adc_digi_output_data_t))
            {
                adc_digi_output_data_t *adc_output = (adc_digi_output_data_t *)&adc_data[i];
                raw_adc = adc_output->type2.data;
                ESP_LOGI(TAG, "ADC: %d, Voltage: %.2fV", raw_adc, ((raw_adc * 3.3) / 4095));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
