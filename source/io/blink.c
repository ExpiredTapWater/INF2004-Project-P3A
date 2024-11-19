#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define DELAY 1000

void led_task()
{   
    while (true) {
        cyw43_arch_gpio_put(0, 1);
        vTaskDelay(pdMS_TO_TICKS(DELAY));
        cyw43_arch_gpio_put(0, 0);
        vTaskDelay(pdMS_TO_TICKS(DELAY));
    }
}