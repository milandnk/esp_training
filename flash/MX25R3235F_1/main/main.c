#include "stdio.h"
#include "string.h"
#include "esp_log.h"
#include "esp_err.h"
#include "MX25R3235F.h"

#define FLASH_OK ESP_OK
#define BUFFER_SIZE 32  // Size of the circular buffer
#define USER_DATA_ADDR 0x008000  // Flash address to store data

typedef struct
{
    char *buf;
    size_t size;
    size_t head;
    size_t tail;
} circ_buf_t;

typedef struct data
{
    circ_buf_t circ_buf;
} data_t;

static const char *TAG = "MX25R3235F";

void circ_buf_init(circ_buf_t *cbuf, size_t size)
{
    cbuf->size = size;
    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->buf = malloc(size);
}

int write_circular_buffer(circ_buf_t *cbuf, const char *data, size_t len)
{
    int i;
    for (i = 0; i < len && i < cbuf->size; i++)
    {
        cbuf->buf[cbuf->head] = data[i];
        cbuf->head = (cbuf->head + 1) % cbuf->size;  
    }
    if (i < cbuf->size)
    {
        cbuf->buf[cbuf->head] = '\0';
        cbuf->head = (cbuf->head + 1) % cbuf->size;  // Wrap around after null-terminating
    }
    return i;  // Return number of characters written
}

// Function to read from circular buffer
int read_circular_buffer(circ_buf_t *cbuf, char *data, size_t len)
{
    int i;
    for (i = 0; i < len && cbuf->tail != cbuf->head; i++)
    {
        data[i] = cbuf->buf[cbuf->tail];
        cbuf->tail = (cbuf->tail + 1) % cbuf->size;  // Wrap around
    }
    // Null-terminate the string after reading
    data[i] = '\0';
    return i;  // Return number of characters read
}

void init(data_t *d1)
{
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

    // Initialize circular buffer
    circ_buf_init(&(d1->circ_buf), BUFFER_SIZE);
}

void write_data(data_t *d1, uint32_t dst_address, const char *data)
{
    ESP_LOGI(TAG, "Writing data to flash...");
    
    // Write data to the circular buffer
    write_circular_buffer(&(d1->circ_buf), data, strlen(data) + 1); // Including null-terminator
    
    int len = strlen(d1->circ_buf.buf) + 1; // Including null-terminator
    esp_err_t result = MX25R3235F_Flash_Write(d1->circ_buf.buf, len, dst_address);
    if (result == FLASH_OK)
    {
        ESP_LOGI(TAG, "Data written to flash successfully!");
    }
    else
    {
        ESP_LOGE(TAG, "Flash write failed with error: %s", esp_err_to_name(result));
    }
}

void read_data(data_t *d1, char *rx_buf, uint32_t dst_address)
{
    ESP_LOGI(TAG, "Reading data from flash...");
    int len = BUFFER_SIZE; // or use actual length of data
    esp_err_t result = MX25R3235F_Flash_Read(d1->circ_buf.buf, len, dst_address);
    if (result == FLASH_OK)
    {
        read_circular_buffer(&(d1->circ_buf), rx_buf, BUFFER_SIZE);
        ESP_LOGI(TAG, "Received data: %s", rx_buf);
    }
    else
    {
        ESP_LOGE(TAG, "Flash read failed with error: %s", esp_err_to_name(result));
    }
}

void app_main(void)
{
    // Allocate memory for receiving buffer
    char *rx_buf = malloc(BUFFER_SIZE);
    data_t d1;

    // Initialize data
    init(&d1);

    // Write data to flash
    write_data(&d1, USER_DATA_ADDR, "data send in string");
    write_data(&d1, USER_DATA_ADDR, "data send 1 in string");

    read_data(&d1, rx_buf, USER_DATA_ADDR);

    // Free allocated memory
    free(d1.circ_buf.buf);
    free(rx_buf);
}