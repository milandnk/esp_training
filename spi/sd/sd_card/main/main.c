#include <stdio.h>
#include <string.h>
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#define PIN_NUM_MISO 13
#define PIN_NUM_MOSI 12
#define PIN_NUM_CLK 15
#define PIN_NUM_CS 14

static const char *TAG = "SD_SPI";

void init_sdcard_spi(void)
{
    // Configure the SPI bus
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI3_HOST; // Using HSPI for SPI communication

    // SPI device (SD card) configuration
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS; // CS pin for SD card
    slot_config.host_id = host.slot;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,    // Not used
        .quadhd_io_num = -1,    // Not used
        .max_transfer_sz = 4000 // Max transfer size
    };

    // Initialize the SPI bus
    esp_err_t ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    ESP_ERROR_CHECK(ret);

    // FAT filesystem mount configuration
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,  // Do not format if mount fails
        .max_files = 5,                   // Max number of open files
        .allocation_unit_size = 16 * 1024 // Allocation size (16KB)
    };

    // Variable to store information about the SD card
    sdmmc_card_t *card;

    // Mount the SD card at the "/sdcard" path
    ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);

}

void create_and_write_file()
{
    FILE *f = fopen("/sdcard/hello.txt", "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing.");
        return;
    }
    fprintf(f, "Hello, writing to the SD card via SPI!\n");
    fclose(f);

    f = fopen("/sdcard/hello.txt", "a");
    for (int i = 0; i < 50; i++)
    {
        fprintf(f, "milan\n");
    }
    fclose(f);
}


void app_main(void)
{
    init_sdcard_spi();
    create_and_write_file();

}
