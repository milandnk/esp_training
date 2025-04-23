#include <stdio.h>
#include "lps22hb.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <driver/i2c.h>

#define I2C_MASTER_SCL_IO 14
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TIMEOUT_MS 1000

static const char *TAG = "lps22hb";

#define I2C_ADDR 0x5C

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

void pressure()
{
    float p;
    while (1)
    {
        lps22hb_read_pressure(&p);
        ESP_LOGI(TAG, "pressure is:%02f", p);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void temperature()
{
    float t;
    while (1)
    {
        lps22hb_read_temperature(&t);
        ESP_LOGI(TAG, "temperature is:%02f", t);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

TaskHandle_t th1 = NULL;
TaskHandle_t th2 = NULL;

void app_main()
{

    init_i2c();
    i2c_scanner();              // check i2c device address
    lps22hb_init();
    lps22hb_who_am_i(I2C_ADDR); // check who am i id is as espected

    BaseType_t xReturn = NULL;
    BaseType_t xReturn1 = NULL;
   

    xReturn = xTaskCreate(pressure, "pressure", 2048, NULL, 2, &th1);
    if (xReturn != pdPASS)
    {
        printf("Failed to create the pressure task!\r\n");
    }

    xReturn1 = xTaskCreate(temperature, "temperature", 2048, NULL, 1, &th2);
    if (xReturn1 != pdPASS)
    {
        printf("Failed to create the temperature task!\r\n");
    }
}