#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "header.h"
#include "semphr.h"

// Define the queue for task-to-task communication
QueueHandle_t received_queue = NULL;
QueueHandle_t commands_queue = NULL;
SemaphoreHandle_t mutex;

// Determines to setup for Access Point or via Hotspot
bool HOTSPOT = true;

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    while(1){
        printf("OVERFLOW from task: %s (Handle: %p)\n", pcTaskName, (void*)xTask);
        sleep_ms(500);
    }
}

void vApplicationMallocFailedHook(void) {
    while(1){
        printf("MALLOC");
        sleep_ms(500);
    }
}

int main(void)
{
    // Initialise everything
    stdio_init_all();
    cyw43_arch_init();
    setup_gpio_motor();
    setup_pwm_motor();
    setup_encoder();

    /* Read state to see what connection mode was selected
    Default button is GP22 (Pulled High)*/
    HOTSPOT = get_connection_mode();

    // Create mutex, queues and handles
    mutex = xSemaphoreCreateMutex();

    received_queue = xQueueCreate(10, MESSAGE_BUFFER);
    commands_queue = xQueueCreate(4, sizeof(int));

    TaskHandle_t core_0_A, core_0_B, core_0_C,core_0_D,core_0_E;
    TaskHandle_t core_1_A, core_1_B, core_1_C;
    TaskHandle_t debug_core;

    // Create blinky task
    xTaskCreate(blink, "LEDTask", 512, NULL, 1, &core_0_A);

    // Create SMP testing task
    int GPIO_PIN = 28;
    xTaskCreate(GPIO_blink, "GPIOTask", 128, &GPIO_PIN, 1, &core_1_A);

    // Run the corresponding connection method based on user's input
    if (HOTSPOT == 0){
        xTaskCreate(start_UDP_server_hotspot, "UDPTask", 1024, NULL, 3, &core_0_B);
    }else{
        xTaskCreate(start_UDP_server_ap, "UDPTask", 1024, NULL, 3, &core_0_C);
    }

    // Create queue consumer task
    xTaskCreate(message_handler, "MessageTask", 1024, NULL, 2, &core_0_D);

    // Create heartbeat task
    xTaskCreate(heartbeat_task, "HearbeatTask", 1024, NULL, 1, &core_0_E);

    // Create motor movement task
    xTaskCreate(motor_task, "MotorTask", 1024, NULL, 1, &core_1_B);

    // Create motor command task
    xTaskCreate(process_motor_commands, "CmdTask", 1024, NULL, 1, &core_1_C);

    //xTaskCreate(encoder_debug_task, "DebugTask", 256, NULL, 1, NULL);

    // Pin handles to cores
    vTaskCoreAffinitySet(core_0_A, (1 << 0)); // Core 0
    vTaskCoreAffinitySet(core_0_B, (1 << 0)); // Core 0
    vTaskCoreAffinitySet(core_0_C, (1 << 0)); // Core 0
    vTaskCoreAffinitySet(core_0_D, (1 << 0)); // Core 0
    vTaskCoreAffinitySet(core_0_E, (1 << 0)); // Core 0
    vTaskCoreAffinitySet(core_1_A, (1 << 1)); // Core 1
    vTaskCoreAffinitySet(core_1_B, (1 << 1)); // Core 1
    vTaskCoreAffinitySet(core_1_C, (1 << 1)); // Core 1

    vTaskStartScheduler();

    while(1){};
}