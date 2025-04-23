#include <stdio.h>
#include "sht21.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <driver/i2c.h>
#include "esp_timer.h"
#include "driver/gpio.h"
#include <sys/time.h>

#define temp_interrpt_time 10000000

#define I2C_MASTER_SCL_IO 18
#define I2C_MASTER_SDA_IO 17
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TIMEOUT_MS 1000

static const char *TAG = "sht21";

void init_i2c()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);                // Config hardware to above structure
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0); // I2C_NUM, mode, slv_rx_buf_len, slv_tx_buf_len, intr_alloc_flags
}

void i2c_scanner()
{
    ESP_LOGI(TAG, "Scanning I2C bus...");

    for (int addr = 1; addr < 127; addr++)
    {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "I2C device found at address: 0x%02X", addr);
        }
    }
    ESP_LOGI(TAG, "I2C scan complete.");
}
void temp_hum()
{
    float t;
    esp_err_t err;
    float h;
    err = sht21_get_temperature(&t);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error reading temperature: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "Temperature: %f", t);
    }
    if (t>=32)
    {
        ESP_LOGE(TAG, "ALERT");
        ESP_LOGW(TAG, "Temperature WAS HIGH UP TO: %f ^CELSCIUS", t);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    

    err = sht21_get_humidity(&h);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error reading humidity: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "Humidity: %f", h);
    }
    if (h>=70)
    {
        ESP_LOGE(TAG, "ALERT");
        ESP_LOGW(TAG, "HUMIDITY WAS HIGH UP TO: %f PERCENTAGE", h);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void app_main()
{

    init_i2c();
    sht21_soft_reset();
    sht21_init(I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ);
    i2c_scanner(); // check i2c device address
    const esp_timer_create_args_t temp_hum_timer_args = {
        .callback = &temp_hum,   // Callback function
        .name = "temp_hum_timer" // Timer name
    };

    esp_timer_handle_t temp_hum_handler;                            // Create handler
    esp_timer_create(&temp_hum_timer_args, &temp_hum_handler);      // Create timer
    esp_timer_start_periodic(temp_hum_handler, temp_interrpt_time); // Start timer with 5-second period
}