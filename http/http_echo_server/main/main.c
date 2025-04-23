// #include <string.h>
// #include <esp_wifi.h>
// #include <esp_event.h>
// #include <nvs_flash.h>
// #include <esp_log.h>
// #include <esp_http_server.h>
// #include "cJSON.h" // Include cJSON library
// #include "driver/gpio.h"

// #define WIFI_SSID "ABCD"
// #define WIFI_PASS "00000000"

// #define LED_GPIO_PIN 35  // Define the GPIO pin for the LED
// static const char *TAG = "http_server";


// void handle_incoming_json(const char *data, int data_len,httpd_req_t *req)
// {
//     cJSON *json = cJSON_ParseWithLength(data, data_len);
//     if (json == NULL)
//     {
//         ESP_LOGE(TAG, "Failed to parse JSON");
//         return;
//     }
//     cJSON *led = cJSON_GetObjectItem(json, "led");
//     if (cJSON_IsString(led) && (led->valuestring != NULL))
//     {
//         ESP_LOGI(TAG, "Received LED command: %s", led->valuestring);
//         if (strcmp(led->valuestring, "on") == 0)
//         {
//             gpio_set_level(LED_GPIO_PIN, 1);
//             ESP_LOGI(TAG, "LED is ON");
//             httpd_resp_send(req, "LED is ON", HTTPD_RESP_USE_STRLEN);
//         }
//         else if (strcmp(led->valuestring, "off") == 0)
//         {
//             gpio_set_level(LED_GPIO_PIN, 0);
//             ESP_LOGI(TAG, "LED is OFF");
//             httpd_resp_send(req, "LED is OFF", HTTPD_RESP_USE_STRLEN);
//         }
//         else
//         {
//             ESP_LOGE(TAG, "Invalid LED command received");
//         }
//     }
//     else
//     {
//         ESP_LOGE(TAG, "Invalid JSON format for 'led'");
//     }
//     cJSON_Delete(json); // Free the allocated memory
// }

// static esp_err_t echo_post_handler(httpd_req_t *req)
// {
//     char *content = calloc(100, sizeof(char)); // Dynamically allocate memory for content
//     if (content == NULL)
//     {
//         printf("Memory allocation failed!\n");
//         httpd_resp_send(req, "FAILURE", HTTPD_RESP_USE_STRLEN);
//         return ESP_FAIL; // Exit if memory allocation fails
//     }

//     // Receive HTTP request content
//     int ret = httpd_req_recv(req, content, 100);
//     if (ret <= 0)
//     {
//         printf("Failed to receive content or connection closed\n");
//         httpd_resp_send(req, "FAILURE", HTTPD_RESP_USE_STRLEN);
//         free(content); // Free allocated memory before returning
//         return ESP_FAIL;
//     }

//     content[ret] = '\0';

//     ESP_LOGI(TAG, "Content received: %s", content);

//     handle_incoming_json(content, ret,req);

//     free(content); // Free memory after use

    
//     return ESP_OK;
// }

// void start_http_server()
// {
//     httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
//     httpd_handle_t server_handle = NULL;
//     httpd_start(&server_handle, &server_config);

//     httpd_uri_t uri_post = {
//         .uri = "/",
//         .method = HTTP_POST,
//         .handler = echo_post_handler,
//         .user_ctx = NULL};

//     httpd_register_uri_handler(server_handle, &uri_post);
// }

// static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
// {
//     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
//     {
//         esp_wifi_connect();
//     }
//     else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
//     {
//         esp_wifi_connect();
//     }
//     else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
//     {
//         start_http_server();
//         ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
//         ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
//     }
// }

// void wifi_init_sta(void)
// {
//     esp_netif_init();
//     esp_event_loop_create_default();
//     esp_netif_create_default_wifi_sta();
//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     esp_wifi_init(&cfg);

//     esp_event_handler_instance_t instance_any_id;
//     esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
//     esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_any_id);

//     wifi_config_t wifi_config = {
//         .sta = {
//             .ssid = WIFI_SSID,
//             .password = WIFI_PASS,
//         },
//     };

//     esp_wifi_set_mode(WIFI_MODE_STA);
//     esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
//     esp_wifi_start();
// }

// void app_main(void)
// {
//     ESP_ERROR_CHECK(nvs_flash_init());

//     gpio_reset_pin(LED_GPIO_PIN);
//     gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT);
//     gpio_set_level(LED_GPIO_PIN, 0); 

//     wifi_init_sta();
// }

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_http_client.h>
#include "cJSON.h"
#include "driver/uart.h"

#define WIFI_SSID "ABCD"
#define WIFI_PASS "00000000"
#define SERVER_URL "http://192.168.27.121:5001/chat"  // Change to actual server IP

#define UART_NUM UART_NUM_0  // Using UART1
#define UART_BUF_SIZE 128    // UART buffer size

#define MAX_HTTP_RESPONSE_SIZE 512  // Define max response size

static const char *TAG = "http_client";
char response_buffer[MAX_HTTP_RESPONSE_SIZE];  // Buffer for HTTP response
int response_length = 0;

// UART Initialization
void uart_init()
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    const int uart_buffer_size = (1024 * 2);
    uart_driver_install(UART_NUM_0, uart_buffer_size, uart_buffer_size, 0, NULL, 0);
}

// HTTP Event Handler
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_DATA:
        if (evt->data_len + response_length < MAX_HTTP_RESPONSE_SIZE)
        {
            memcpy(response_buffer + response_length, evt->data, evt->data_len);
            response_length += evt->data_len;
            response_buffer[response_length] = '\0';  // Null-terminate
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "%s", response_buffer);
        response_length = 0;  // Reset buffer
        break;
    default:
        break;
    }
    return ESP_OK;
}

// Function to Send HTTP Request with UART Data
void send_http_request(char *message)
{
    ESP_LOGI(TAG, "Sending HTTP request...");

    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .event_handler = _http_event_handler};

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Construct JSON payload with UART data
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "message", message);
    char *json_data = cJSON_Print(json);
    cJSON_Delete(json);  // Free JSON object

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_data, strlen(json_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP POST Status = %d", esp_http_client_get_status_code(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    free(json_data);
    esp_http_client_cleanup(client);
}

// Wi-Fi Event Handler
void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(TAG, "Connected to Wi-Fi");
    }
}

// Initialize Wi-Fi
void wifi_init_sta()
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
}

// UART Task: Reads UART Data and Sends HTTP Request
void uart_task(void *arg)
{
    char uart_data[UART_BUF_SIZE];
    while (1)
    {
        int len = uart_read_bytes(UART_NUM, (uint8_t *)uart_data, sizeof(uart_data) - 1, 500 / portTICK_PERIOD_MS);
        if (len > 0)
        {
            uart_data[len] = '\0';  // Null-terminate the string
            ESP_LOGI(TAG, "Received UART Data: %s", uart_data);  // Print to log
            send_http_request(uart_data);  // Send data to server
        }
        vTaskDelay(pdMS_TO_TICKS(500));  // Delay before next read
    }
}


void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
    uart_init();

    // Create a FreeRTOS task to handle UART
    xTaskCreate(uart_task, "uart_task", 4096, NULL, 5, NULL);
}
