#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define BLINK_GPIO GPIO_NUM_38

void app_main(void)
{

    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    while (1)
    {
        gpio_set_level(BLINK_GPIO, 0);
        printf("off");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        
        gpio_set_level(BLINK_GPIO, 1);
         printf("on");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}