#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "spi_flash_mmap.h"
#include "esp_flash.h"
#include "esp_flash_spi_init.h"
#include "MX25R3235F.h"

#ifndef SPI_FLASH_SEC_SIZE
#define SPI_FLASH_SEC_SIZE 4096
#endif

#define FLASH_OK ESP_OK
#define FLASH_ERROR ESP_FAIL

static const char *TAG = "MX25R3235F";

esp_flash_t *ext_flash = NULL;

#define QSPI_FLASH_IO0 11
#define QSPI_FLASH_IO1 13
#define QSPI_FLASH_IO2 14
#define QSPI_FLASH_IO3 9
#define QSPI_FLASH_CS 10
#define QSPI_FLASH_CLK 12

esp_err_t MX25R3235F_Flash_Init(void)
{
    esp_err_t err_code;

    spi_bus_config_t buscfg = {
        .mosi_io_num = QSPI_FLASH_IO0,
        .miso_io_num = QSPI_FLASH_IO1,
        .sclk_io_num = QSPI_FLASH_CLK,
        .quadwp_io_num = QSPI_FLASH_IO2,
        .quadhd_io_num = QSPI_FLASH_IO3,
        .max_transfer_sz = 64,
    };

    const esp_flash_spi_device_config_t device_config = {
        .host_id = SPI2_HOST,
        .cs_id = 0,
        .cs_io_num = QSPI_FLASH_CS,
        .io_mode = SPI_FLASH_FASTRD,
        .freq_mhz = 20,
    };

    ESP_LOGI(TAG, "MOSI: %2d   MISO: %2d   SCLK: %2d   CS: %2d",
             buscfg.mosi_io_num, buscfg.miso_io_num,
             buscfg.sclk_io_num, device_config.cs_io_num);

    // Initialize the SPI bus
    ESP_LOGI(TAG, "DMA CHANNEL: %d", SPI_DMA_CH_AUTO);
    err_code = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (err_code == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGW(TAG, "SPI bus already initialized, proceeding with device configuration.");
    }
    else if (err_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(err_code));
        return err_code;
    }

    err_code = spi_bus_add_flash_device(&ext_flash, &device_config);
    if (err_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add flash device: %s", esp_err_to_name(err_code));
        spi_bus_free(SPI2_HOST);
        return err_code;
    }

    err_code = esp_flash_init(ext_flash);
    if (err_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize flash: %s (0x%x)", esp_err_to_name(err_code), err_code);
        spi_bus_remove_device((spi_device_handle_t)ext_flash);
        spi_bus_free(SPI2_HOST);
        return err_code;
    }

    uint32_t id;
    err_code = esp_flash_read_id(ext_flash, &id);
    if (err_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read flash ID: %s", esp_err_to_name(err_code));
        return err_code;
    }

    ESP_LOGI(TAG, "Initialized external Flash, size=%" PRIu32 " KB, ID=0x%" PRIx32, ext_flash->size / 1024, id);

    return FLASH_OK;
}

esp_err_t MX25R3235F_Flash_DeInit(void)
{
    if (!ext_flash)
    {
        ESP_LOGE(TAG, "Flash device is not initialized.");
        return FLASH_ERROR;
    }

    esp_err_t err_code = spi_bus_remove_device((spi_device_handle_t)ext_flash);
    if (err_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to remove flash device: %s", esp_err_to_name(err_code));
        return FLASH_ERROR;
    }

    err_code = spi_bus_free(SPI2_HOST);
    if (err_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to free SPI bus: %s", esp_err_to_name(err_code));
        return FLASH_ERROR;
    }

    ext_flash = NULL;
    ESP_LOGI(TAG, "MX25R3235F flash de-initialized.");
    return FLASH_OK;
}

esp_err_t MX25R3235F_Flash_Erase(uint32_t start_address, qspi_erase_len len)
{
    esp_err_t err_code;

    if (!ext_flash)
    {
        ESP_LOGE(TAG, "Flash is not initialized.");
        return FLASH_ERROR;
    }

    ESP_LOGI(TAG, "Erasing flash memory at address: 0x%08" PRIx32 ", length: %d", start_address, len);

    switch (len)
    {
    case QSPI_ERASE_LEN_4KB:
        err_code = esp_flash_erase_region(ext_flash, start_address, SPI_FLASH_SEC_SIZE);
        break;
    case QSPI_ERASE_LEN_64KB:
        err_code = esp_flash_erase_region(ext_flash, start_address, 64 * 1024);
        break;
    case QSPI_ERASE_LEN_ALL:
        err_code = esp_flash_erase_chip(ext_flash);
        break;
    default:
        ESP_LOGE(TAG, "Invalid erase length specified.");
        return ESP_ERR_INVALID_ARG;
    }

    if (err_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Erase operation failed: %s", esp_err_to_name(err_code));
    }
    else
    {
        ESP_LOGI(TAG, "Erase operation successful.");
    }

    return err_code;
}

esp_err_t MX25R3235F_Flash_Write(const void *p_tx_buffer, size_t tx_buffer_length, uint32_t dst_address)
{
    if (!ext_flash)
    {
        ESP_LOGE(TAG, "Flash is not initialized.");
        // return FLASH_ERROR;
    }

    ESP_LOGI(TAG, "Writing to flash memory at address: 0x%08" PRIx32 ", length: %zu", dst_address, tx_buffer_length);

    esp_err_t err_code = MX25R3235F_Flash_Erase(dst_address, QSPI_ERASE_LEN_4KB);
    if (err_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Flash erase failed: %s", esp_err_to_name(err_code));
        return err_code;
    }

    err_code = esp_flash_write(ext_flash, p_tx_buffer, dst_address, tx_buffer_length);
    if (err_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Flash write failed: %s", esp_err_to_name(err_code));
    }
    else
    {
        ESP_LOGI(TAG, "Flash write successful.");
    }

    return err_code;
    // return 0;
}

esp_err_t MX25R3235F_Flash_Read(void *p_rx_buffer, size_t rx_buffer_length, uint32_t src_address)
{
    if (!ext_flash)
    {
        ESP_LOGE(TAG, "Flash is not initialized.");
        return FLASH_ERROR;
    }

    ESP_LOGI(TAG, "Reading from flash memory at address: 0x%08" PRIx32 ", length: %zu", src_address, rx_buffer_length);

    esp_err_t err_code = esp_flash_read(ext_flash, p_rx_buffer, src_address, rx_buffer_length);
    if (err_code != ESP_OK)
    {
        ESP_LOGE(TAG, "Flash read failed: %s", esp_err_to_name(err_code));
    }
    else
    {
        ESP_LOGI(TAG, "Flash read successful.");
    }

    return err_code;
}

// #include "esp_log.h"
// #include "esp_err.h"
// #include "driver/spi_common.h"
// #include "driver/spi_master.h"
// #include "spi_flash_mmap.h"
// #include "esp_flash.h"
// #include "esp_flash_spi_init.h"
// #include "MX25R3235F.h"

// #define FLASH_OK ESP_OK

// static const char *TAG = "MX25R3235F";

// esp_flash_t *ext_flash;

// esp_err_t MX25R3235F_Flash_Init(void)
// {
//     esp_err_t err_code;
//     spi_bus_config_t buscfg = {
//         .mosi_io_num = QSPI_FLASH_IO0,   // IO0
//         .miso_io_num = QSPI_FLASH_IO1,   // IO1
//         .sclk_io_num = QSPI_FLASH_CLK,   // CLK
//         .quadwp_io_num = QSPI_FLASH_IO2, // IO2 (WP)
//         .quadhd_io_num = QSPI_FLASH_IO3, // IO3 (HD)
//         .max_transfer_sz = 64,
//     };

//     // Initialize the SPI bus
//     err_code = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
//     if (err_code == ESP_ERR_INVALID_STATE)
//     {
//         ESP_LOGW(TAG, "SPI bus already initialized, proceeding with device configuration");
//     }
//     else if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(err_code));
//         return err_code;
//     }

//     esp_flash_spi_device_config_t device_config = {
//         .host_id = SPI3_HOST,
//         .cs_id = 0,
//         .cs_io_num = QSPI_FLASH_CS, // CS
//         .io_mode = SPI_FLASH_QIO,
//         .speed = ESP_FLASH_40MHZ,
//     };

//     // Add the external flash device to the SPI bus
//     err_code = spi_bus_add_flash_device(&ext_flash, &device_config);
//     if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Failed to add external flash device: %s", esp_err_to_name(err_code));
//         spi_bus_free(SPI3_HOST); // Free the SPI bus if device addition fails
//         return err_code;
//     }

//     // Initialize the external flash
//     err_code = esp_flash_init(ext_flash);
//     if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Failed to initialize external flash: %s", esp_err_to_name(err_code));
//         spi_bus_remove_device(ext_flash); // Remove the device if flash init fails
//         spi_bus_free(SPI3_HOST);          // Free the SPI bus if flash init fails
//         return err_code;
//     }

//     ESP_LOGI(TAG, "MX25R3235F external flash initialized successfully.");
//     return FLASH_OK;
// }
// esp_err_t MX25R3235F_Flash_DeInit(void)
// {
//     return spi_bus_remove_device(ext_flash);
// }

// esp_err_t MX25R3235F_Flash_Erase(uint32_t start_address, qspi_erase_len len)
// {
//     esp_err_t err_code;

//     switch (len)
//     {
//     case QSPI_ERASE_LEN_4KB:
//         err_code = esp_flash_erase_region(ext_flash, start_address, SPI_FLASH_SEC_SIZE);
//         break;
//     case QSPI_ERASE_LEN_64KB:
//         err_code = esp_flash_erase_region(ext_flash, start_address, 64 * 1024);
//         break;
//     case QSPI_ERASE_LEN_ALL:
//         err_code = esp_flash_erase_chip(ext_flash);
//         break;
//     default:
//         err_code = ESP_ERR_INVALID_ARG;
//         break;
//     }

//     if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Erase operation failed: %s", esp_err_to_name(err_code));
//     }

//     return err_code;
// }

// esp_err_t MX25R3235F_Flash_Write(void *p_tx_buffer, size_t tx_buffer_length, uint32_t dst_address)
// {
//     esp_err_t err_code = esp_flash_write(ext_flash, p_tx_buffer, dst_address, tx_buffer_length);
//     if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Flash write operation failed: %s", esp_err_to_name(err_code));
//     }
//     return err_code;
// }

// esp_err_t MX25R3235F_Flash_Read(void *p_rx_buffer, size_t rx_buffer_length, uint32_t src_address)
// {
//     esp_err_t err_code = esp_flash_read(ext_flash, p_rx_buffer, src_address, rx_buffer_length);
//     if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Flash read operation failed: %s", esp_err_to_name(err_code));
//     }
//     return err_code;
// }
// #include "esp_log.h"
// #include "esp_err.h"
// #include "driver/spi_common.h"
// #include "driver/spi_master.h"
// #include "spi_flash_mmap.h"
// #include "esp_flash.h"
// #include "esp_flash_spi_init.h"
// #include "MX25R3235F.h"

// #define FLASH_OK ESP_OK

// static const char *TAG = "MX25R3235F";

// esp_flash_t *ext_flash;

// esp_err_t MX25R3235F_Flash_Init(void)
// {
//     esp_err_t err_code;
//     spi_bus_config_t buscfg = {
//         .mosi_io_num = QSPI_FLASH_IO0,   // IO0
//         .miso_io_num = QSPI_FLASH_IO1,   // IO1
//         .sclk_io_num = QSPI_FLASH_CLK,   // CLK
//         .quadwp_io_num = QSPI_FLASH_IO2, // IO2 (WP)
//         .quadhd_io_num = QSPI_FLASH_IO3, // IO3 (HD)
//         .max_transfer_sz = 64,
//     };

//     // Initialize the SPI bus
//     err_code = spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
//     if (err_code == ESP_ERR_INVALID_STATE)
//     {
//         ESP_LOGW(TAG, "SPI bus already initialized, proceeding with device configuration");
//     }
//     else if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(err_code));
//         return err_code;
//     }

//     spi_device_interface_config_t devcfg = {
//         .clock_speed_hz = 40 * 1000 * 1000, // 40 MHz for QSPI
//         .mode = 0,                          // SPI mode 0
//         .spics_io_num = QSPI_FLASH_CS,      // Chip Select
//         .queue_size = 7,
//         .flags = SPI_DEVICE_HALFDUPLEX, // Optional flag for QSPI
//     };

//     spi_device_handle_t handle;
//     err_code = spi_bus_add_device(SPI3_HOST, &devcfg, &handle);
//     if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Failed to add SPI device");
//         return err_code;
//     }
//     err_code = esp_flash_init(ext_flash);
//     if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Failed to initialize external flash: %s", esp_err_to_name(err_code));
//         spi_bus_remove_device(ext_flash); // Remove the device if flash init fails
//         spi_bus_free(SPI3_HOST);          // Free the SPI bus if flash init fails
//         return err_code;
//     }

//     ESP_LOGI(TAG, "MX25R3235F external flash initialized successfully.");
//     return FLASH_OK;
// }

// esp_err_t MX25R3235F_Flash_DeInit(void)
// {
//     esp_err_t err_code = spi_bus_remove_device(ext_flash);
//     if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Failed to remove external flash device: %s", esp_err_to_name(err_code));
//         return FLASH_ERROR;
//     }

//     err_code = spi_bus_free(SPI3_HOST);
//     if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Failed to free SPI bus: %s", esp_err_to_name(err_code));
//         return FLASH_ERROR;
//     }

//     return FLASH_OK;
// }

// esp_err_t MX25R3235F_Flash_Erase(uint32_t start_address, qspi_erase_len len)
// {
//     esp_err_t err_code;

//     switch (len)
//     {
//     case QSPI_ERASE_LEN_4KB:
//         err_code = esp_flash_erase_region(ext_flash, start_address, SPI_FLASH_SEC_SIZE);
//         break;
//     case QSPI_ERASE_LEN_64KB:
//         err_code = esp_flash_erase_region(ext_flash, start_address, 64 * 1024);
//         break;
//     case QSPI_ERASE_LEN_ALL:
//         err_code = esp_flash_erase_chip(ext_flash);
//         break;
//     default:
//         err_code = ESP_ERR_INVALID_ARG;
//         break;
//     }

//     if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Erase operation failed: %s", esp_err_to_name(err_code));
//     }

//     return err_code;
// }

// esp_err_t MX25R3235F_Flash_Write(void *p_tx_buffer, size_t tx_buffer_length, uint32_t dst_address)
// {
//     esp_err_t err_code = esp_flash_write(ext_flash, p_tx_buffer, dst_address, tx_buffer_length);
//     if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Flash write operation failed: %s", esp_err_to_name(err_code));
//     }
//     return err_code;
// }

// esp_err_t MX25R3235F_Flash_Read(void *p_rx_buffer, size_t rx_buffer_length, uint32_t src_address)
// {
//     esp_err_t err_code = esp_flash_read(ext_flash, p_rx_buffer, src_address, rx_buffer_length);
//     if (err_code != ESP_OK)
//     {
//         ESP_LOGE(TAG, "Flash read operation failed: %s", esp_err_to_name(err_code));
//     }
//     return err_code;
// }