#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "header.h"

void sample_ultrasonic_task(void *pvParameters);

// Define the queue for task-to-task communication
QueueHandle_t received_queue = NULL;
QueueHandle_t commands_queue = NULL;
//SemaphoreHandle_t mutex;

// Create task handles globally (for use in task manager)
TaskHandle_t LED_T = NULL;
TaskHandle_t GPIO_T = NULL;
TaskHandle_t UDP_T = NULL;
TaskHandle_t Message_T = NULL;
TaskHandle_t Command_T = NULL;
TaskHandle_t Heartbeat_T = NULL;
TaskHandle_t Motor_T = NULL;
TaskHandle_t TaskManager_T = NULL;
TaskHandle_t TestHandle_1 = NULL;
TaskHandle_t TestHandle_2 = NULL;

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
    setup_interrupts();

    /* Read state to see what connection mode was selected
    Default button is GP22 (Pulled High)*/
    HOTSPOT = get_connection_mode();

    // Create queues
    received_queue = xQueueCreate(10, sizeof(uint8_t));
    commands_queue = xQueueCreate(4, sizeof(uint8_t));

    // --------------- TASK CREATION ---------------

    // Create blinky task
    xTaskCreate(blink, "LEDTask", 512, NULL, 1, &LED_T);

    // Create SMP testing task
    int GPIO_PIN = 28;
    xTaskCreate(GPIO_blink, "GPIOTask", 512, &GPIO_PIN, 1, &GPIO_T);

    // Run the corresponding connection method based on user's input
    if (HOTSPOT == 0){
        xTaskCreate(start_UDP_server_hotspot, "UDPTask", 1024, NULL, 3, &UDP_T);
    }else{
        xTaskCreate(start_UDP_server_ap, "UDPTask", 1024, NULL, 3, &UDP_T);
    }

    // Create queue consumer task
    xTaskCreate(message_handler, "MessageTask", 1024, NULL, 2, &Message_T);

    // Create heartbeat task
    xTaskCreate(heartbeat_task, "HearbeatTask", 1024, NULL, 1, &Heartbeat_T);

    // Create motor movement task
    xTaskCreate(motor_task, "MotorTask", 1024, NULL, 3, &Motor_T);

    // Create motor command task
    xTaskCreate(process_motor_commands, "CmdTask", 1024, NULL, 1, &Command_T);

    //xTaskCreate(encoder_debug_task, "DebugTask", 256, NULL, 1, &TestHandle_1);

    xTaskCreate(sample_ultrasonic_task, "UltTask", 256, NULL, 1, &TestHandle_2);

    xTaskCreate(sample_ir_task, "IRTask", 256, NULL, 1, &TestHandle_1);

    xTaskCreate(task_manager, "TMTask", 256, NULL, 1, &TaskManager_T);

    // Pin handles to core 0
    //vTaskCoreAffinitySet(LED_T, (1 << 0));
    //vTaskCoreAffinitySet(UDP_T, (1 << 0));
    //vTaskCoreAffinitySet(Message_T, (1 << 0));
    //vTaskCoreAffinitySet(Heartbeat_T, (1 << 0));
    //vTaskCoreAffinitySet(TaskManager_T, (1 << 0));

    // Pin handles to core 1
    //vTaskCoreAffinitySet(GPIO_T, (1 << 1));
    //vTaskCoreAffinitySet(Motor_T, (1 << 1));
    //vTaskCoreAffinitySet(Command_T, (1 << 1));

    //vTaskCoreAffinitySet(TestHandle_1, (1 << 1)); // Core 1
    //vTaskCoreAffinitySet(TestHandle_2, (1 << 1)); // Core 1

    vTaskStartScheduler();

    while(1){};
}