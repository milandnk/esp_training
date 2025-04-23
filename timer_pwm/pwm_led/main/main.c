#include <stdio.h>
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_PIN 2
void app_main(void)
{

    ledc_timer_config_t pwm_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,   // high spped mode
        .timer_num = LEDC_TIMER_0,           // set timer num
        .duty_resolution = LEDC_TIMER_8_BIT, // 8 bit duty cycle resolution
        .freq_hz = 26000,                     // pwm freq
        .clk_cfg = LEDC_AUTO_CLK             // auto clk source select
    };
    ledc_timer_config(&pwm_timer);

    ledc_channel_config_t pwm_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0,           // init duty
        .gpio_num = LED_PIN, // led pin
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0};
    ledc_channel_config(&pwm_channel);

    while (1)
    {
        // for (int duty_cycle = 0; duty_cycle <= 255; duty_cycle++)
        // {
        //     ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_cycle); // set duty cycle
        //     ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);          // update duty cycle
        //     printf("duty cycle is: %d\n",duty_cycle);
        //     vTaskDelay(30 / portTICK_PERIOD_MS);
        // }
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 192); // set duty cycle
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);          // update duty cycle
    }
}
