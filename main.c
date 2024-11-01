#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "header.h"
#include "semphr.h"

SemaphoreHandle_t mutex;

int main(void)
{
    stdio_init_all();
    cyw43_arch_init();

    mutex = xSemaphoreCreateMutex();

    // Create Task Handles
    TaskHandle_t core_0;
    TaskHandle_t core_1;

    // Create blinky task
    xTaskCreate(blink, "LED_Task", 128, NULL, 1, &core_0);

    // Create SMP testing task
    int GPIO_PIN = 28;
    xTaskCreate(GPIO_blink, "GPIO_Task", 128, &GPIO_PIN, 1, &core_1);

    // Pin Tasks
    vTaskCoreAffinitySet(core_0, (1 << 0)); // Core 0
    vTaskCoreAffinitySet(core_1, (1 << 1)); // Core 1

    vTaskStartScheduler();

    while(1){};
}