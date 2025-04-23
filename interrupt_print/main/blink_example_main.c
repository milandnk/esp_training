#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define BOOT_BUTTON_GPIO GPIO_NUM_0

volatile int i = 0;
volatile bool k = true;

void IRAM_ATTR boot_button_isr_handler(void *arg)
{
    k = true;
}

void app_main(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE; // falling edge
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BOOT_BUTTON_GPIO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // Install ISR service
    gpio_install_isr_service(0);

    // Attach the ISR handler//////////////number,func,arg
    gpio_isr_handler_add(BOOT_BUTTON_GPIO, boot_button_isr_handler, NULL);

    while (1)
    {
        if (k)
        {
            printf("button pressed");
        }
        else
        {
            printf("button not pressed");
        }
        k = false;
        printf("\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
