#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "semphr.h"

#define DELAY 1000

extern SemaphoreHandle_t mutex;

/* For debug: Ensures only one task from either cores can print at one time
Try to ensure printf statements only appear on tasks pinned to core 0 */

void vGuardedPrint(char *out){
    xSemaphoreTake(mutex, portMAX_DELAY);
    puts(out);
    xSemaphoreGive(mutex);
}

// Debug task to print the core the task calling it is running from
void print_pinned_core(void){
    char *task_name = pcTaskGetName(NULL);
    char out[25];

    sprintf(out, "%s on Core: %d", task_name, get_core_num());
    vGuardedPrint(out);

}

// Simple Task to constantly blink the built in LED
void blink(void *pvParameters)
{   

    while (true) {
        cyw43_arch_gpio_put(0, 1);
        vTaskDelay(pdMS_TO_TICKS(DELAY));
        print_pinned_core();
        cyw43_arch_gpio_put(0, 0);
        vTaskDelay(pdMS_TO_TICKS(DELAY));
    }
}

// Additional sample task to blink a GPIO LED for testing SMP
void GPIO_blink(void *param) {
    int GPIO_PIN = *((int *)param);

    gpio_init(GPIO_PIN);
    gpio_set_dir(GPIO_PIN, GPIO_OUT);

    while (true) {
        gpio_put(GPIO_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(DELAY));
        print_pinned_core();
        gpio_put(GPIO_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(DELAY));
    }
}