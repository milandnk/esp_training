// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"

// #define QUEUE_LENGTH 10
// #define QUEUE_ITEM_SIZE sizeof(int)

// QueueHandle_t queue;

// void sender_task(void *param) {
//     int task_id = (int)param;
//     int data = task_id * 10;  // Unique data for each task

//     while (1) {
//         if (xQueueSend(queue, &data, pdMS_TO_TICKS(100)) == pdPASS) {
//             printf("Sender Task %d sent: %d\n", task_id, data);
//         } else {
//             printf("Sender Task %d failed to send\n", task_id);
//         }
//         vTaskDelay(pdMS_TO_TICKS(10));  // Delay before sending again
//     }
// }

// void receiver_task(void *param) {
//     int received_data;

//     while (1) {
//         if (xQueueReceive(queue, &received_data, portMAX_DELAY)) {
//             printf("Receiver received: %d\n", received_data);
//         }
//     }
// }

// void app_main() {
//     queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
//     if (queue == NULL) {
//         printf("Failed to create queue\n");
//         return;
//     }

//     // Create 5 sender tasks
//     for (int i = 1; i <= 5; i++) {
//         char task_name[20];
//         snprintf(task_name, sizeof(task_name), "SenderTask%d", i);
//         xTaskCreate(sender_task, task_name, 2048, (void *)i, 1, NULL);
//     }

//     // Create receiver task
//     xTaskCreate(receiver_task, "ReceiverTask", 2048, NULL, 2, NULL);
// }

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <freertos/semphr.h>
#include "esp_log.h" // Include ESP logging

#define QUEUE_LENGTH 20
#define QUEUE_ITEM_SIZE sizeof(int)
int data[10];
static const char *TAG = "QueueExample"; // Define a tag for logging

QueueHandle_t queue;
SemaphoreHandle_t GlobalHandler = NULL;

void sender_task(void *param)
{
    int task_id = (int)param;

    while (1)
    {
        if (xSemaphoreTake(GlobalHandler, 250) == pdTRUE)
        {
            for (int i = 1; i <= 5; i++)
            {
                data[i] = i * 10; // Unique data for each task

                if (xQueueSend(queue, &data[i], pdMS_TO_TICKS(100)) == pdPASS)
                {
                    printf("Sender Task %d sent: %d\t", task_id, data[i]);
                    for (int i = 1; i <= 5; i++)
                    {
                        printf("%d ", data[i]);
                    }
                    printf("\n");
                }
                else
                {
                    ESP_LOGW(TAG, "Sender Task %d failed to send", task_id);
                    // vTaskDelay(pdMS_TO_TICKS(100)); // Small delay before retrying
                }
            }
            xSemaphoreGive(GlobalHandler);
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Send data every 500ms
    }
}

void receiver_task(void *param)
{
    int received_data;

    while (1)
    { // Run continuously
        if (xSemaphoreTake(GlobalHandler, 250) == pdTRUE)
        {
            if (xQueueReceive(queue, &received_data, portMAX_DELAY))
            {
                printf("Receiver received: %d\t", received_data);
                for (int i = 1; i <= 5; i++)
                {
                    if (data[i] == received_data)
                    {
                        data[i] = 0;
                    }
                    
                    printf("%d ", data[i]);
                }
                printf("\n");
            }
            else
            {
                ESP_LOGW(TAG, "Receiver failed to receive");
            }
            // if ( received_data == 50)
            // {
            xSemaphoreGive(GlobalHandler);
            /* code */
            // }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay before trying again
    }
}

void app_main()
{
    GlobalHandler = xSemaphoreCreateMutex();
    if (GlobalHandler == NULL)
    {
        printf("Failed to Create Mutex\r\n");
    }
    esp_log_level_set(TAG, ESP_LOG_INFO); // Set log level

    queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    if (queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create queue");
        return;
    }

    // Create 5 sender tasks
    int i = 1;
    char task_name[20];
    snprintf(task_name, sizeof(task_name), "SenderTask%d", i);
    BaseType_t xReturned = xTaskCreate(sender_task, task_name, 2048, (void *)i, 5, NULL); // Increased priority
    if (xReturned == pdPASS)
    {
        ESP_LOGI(TAG, "Created task %s", task_name);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to create task %s", task_name);
    }
    // }

    // Create receiver task
    xReturned = xTaskCreate(receiver_task, "ReceiverTask", 2048, NULL, 4, NULL); // Slightly lower priority
    if (xReturned == pdPASS)
    {
        ESP_LOGI(TAG, "Created task ReceiverTask");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to create task ReceiverTask");
    }
}
