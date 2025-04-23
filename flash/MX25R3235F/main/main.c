// // #include "stdio.h"
// // #include "string.h"
// // #include "esp_log.h"
// // #include "esp_err.h"
// // #include "MX25R3235F.h"

// // #define FLASH_OK ESP_OK

// // typedef struct data
// // {
// //     char *buf;
// // } data_t;

// // #define USER_DATA_ADDR 0x00008000

// // static const char *TAG = "MX25R3235F";

// // void init(data_t *d1)
// // {

// //     strcpy(d1->buf, "hello");

// //     ESP_LOGI(TAG, "Starting flash initialization...");

// //     esp_err_t result = MX25R3235F_Flash_Init();
// //     if (result == FLASH_OK)
// //     {
// //         ESP_LOGI(TAG, "Flash initialization successful!");
// //     }
// //     else
// //     {
// //         ESP_LOGE(TAG, "Flash initialization failed with error: %s", esp_err_to_name(result));
// //     }
// // }
// // void write_data(data_t *d1)
// // {
// //     ESP_LOGI(TAG, "write!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
// //     MX25R3235F_Flash_Write(((char *)(d1->buf)), strlen(d1->buf) + 1, USER_DATA_ADDR);
// // }
// // void read_data(char *rx_buf)
// // {
// //     ESP_LOGI(TAG, "read!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
// //     MX25R3235F_Flash_Read(rx_buf, 32, USER_DATA_ADDR);
// //     ESP_LOGE(TAG, "received data is:%s",rx_buf);
// // }
// // void app_main(void)
// // {
// //     char *rx_buf = malloc(32);;
// //     data_t d1;
// //     d1.buf = malloc(32); // Allocate 32 bytes
// //     if (d1.buf == NULL)
// //     {
// //         ESP_LOGE(TAG, "Memory allocation failed!");
// //         return;
// //     }
// //     init(&d1);
// //     write_data(&d1);
// //     read_data(&rx_buf);
// // }
// #include "stdio.h"
// #include "string.h"
// #include "stdlib.h"
// #include "esp_log.h"
// #include "esp_err.h"
// #include "MX25R3235F.h"

// #define FLASH_OK ESP_OK

// typedef struct data
// {
//     char *buf;
// } data_t;

// #define USER_DATA_ADDR 0x00008000

// static const char *TAG = "MX25R3235F";

// void init(data_t *d1)
// {
//     if (d1->buf == NULL)
//     {
//         ESP_LOGE(TAG, "Buffer not allocated!");
//         return;
//     }

//     strcpy(d1->buf, "hello");

//     ESP_LOGI(TAG, "Starting flash initialization...");
//     esp_err_t result = MX25R3235F_Flash_Init();
//     if (result == FLASH_OK)
//     {
//         ESP_LOGI(TAG, "Flash initialization successful!");
//     }
//     else
//     {
//         ESP_LOGE(TAG, "Flash initialization failed with error: %s", esp_err_to_name(result));
//     }
// }

// void app_main(void)
// {
//     char *rx_buf = malloc(32); // Allocate memory for receiving buffer
//     if (rx_buf == NULL)
//     {
//         ESP_LOGE(TAG, "Memory allocation for rx_buf failed!");
//         return;
//     }

//     data_t d1;
//     d1.buf = malloc(32); // Allocate memory for data buffer
//     if (d1.buf == NULL)
//     {
//         ESP_LOGE(TAG, "Memory allocation for d1.buf failed!");
//         free(rx_buf); // Free rx_buf before returning
//         return;
//     }

//     init(&d1);

//     ESP_LOGI(TAG, "Writing data to flash...");
//     MX25R3235F_Flash_Write(d1.buf, strlen(d1.buf) + 1, USER_DATA_ADDR);


//     ESP_LOGI(TAG, "Reading data from flash...");
//     MX25R3235F_Flash_Read(rx_buf, 32, USER_DATA_ADDR);
//     ESP_LOGI(TAG, "Received data is %s!!!!!!!!!!!!!!!!!!!!!!",rx_buf);
//     // Free allocated memory
//     free(rx_buf);
//     free(d1.buf);

//     ESP_LOGI(TAG, "Program completed!");
// }
#include "stdio.h"
#include "string.h"
#include "esp_log.h"
#include "esp_err.h"
#include "MX25R3235F.h"

#define FLASH_OK ESP_OK

typedef struct data
{
    char *buf;
} data_t;

typedef struct data_id
{
    char *id;
} data_id_t;

#define USER_DATA_ADDR 0x008000
#define USER_DATA_ID_ADDR 0x000000

static const char *TAG = "MX25R3235F";

void init(data_t *d1, data_id_t *d2)
{
    if (d1->buf == NULL || d2->id == NULL)
    {
        ESP_LOGE(TAG, "Buffer or ID not allocated.");
        return;
    }

    strcpy(d1->buf, "data");
    strcpy(d2->id, "1");

    ESP_LOGI(TAG, "Starting flash initialization...");

    esp_err_t result = MX25R3235F_Flash_Init();
    if (result == FLASH_OK)
    {
        ESP_LOGI(TAG, "Flash initialization successful!");
    }
    else    
    {
        ESP_LOGE(TAG, "Flash initialization failed with error: %s", esp_err_to_name(result));
    }
}

void write_data(data_t *d1, uint32_t dst_address)
{
    ESP_LOGI(TAG, "Writing data to flash...");
    esp_err_t result = MX25R3235F_Flash_Write((char *)(d1->buf), strlen(d1->buf) + 1, dst_address);
    if (result == FLASH_OK)
    {
        ESP_LOGI(TAG, "Data written to flash successfully!");
    }
    else
    {
        ESP_LOGE(TAG, "Flash write failed with error: %s", esp_err_to_name(result));
    }
}

void write_data_id(data_id_t *d2, uint32_t dst_address)
{

    ESP_LOGI(TAG, "Writing ID to flash...");
    esp_err_t result = MX25R3235F_Flash_Write((char *)(d2->id), strlen(d2->id) + 1, dst_address);
    if (result == FLASH_OK)
    {
        ESP_LOGI(TAG, "ID written to flash successfully!");
    }
    else
    {
        ESP_LOGE(TAG, "Flash write ID failed with error: %s", esp_err_to_name(result));
    }
}

void read_data(char *rx_buf, uint32_t dst_address)
{
    ESP_LOGI(TAG, "Reading data from flash...");
    esp_err_t result = MX25R3235F_Flash_Read(rx_buf, 32, dst_address);
    if (result == FLASH_OK)
    {
        ESP_LOGE(TAG, "Received data: %s", rx_buf);
    }
    else
    {
        ESP_LOGE(TAG, "Flash read failed with error: %s", esp_err_to_name(result));
    }
}

void app_main(void)
{
    char *rx_buf = malloc(32);
    char *rx_buf1 = malloc(32);

    data_t d1;
    d1.buf = malloc(32);
    data_id_t d2;
    d2.id = malloc(32);

    init(&d1, &d2);

    // Write and read data
    write_data(&d1, USER_DATA_ADDR);
    read_data(rx_buf, USER_DATA_ADDR);

    // Write and read data ID
    write_data_id(&d2, USER_DATA_ID_ADDR);
    read_data(rx_buf1, USER_DATA_ID_ADDR);

    // Free allocated memory
    free(d1.buf);
    free(d2.id);
    free(rx_buf);
    free(rx_buf1);

}
