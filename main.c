#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "header.h"
#include "semphr.h"

// From io_handler.c
void setup_buttons();
void message_handler();
void send_commands(void *pvParameters);

// From display.c
void setup_display();
void display_task();

// From mpu6050.c
void setup_sensor();
void accelerometer_task();
void sample_accelerometer_task();

// From wifi.c
void wifi_task();

TaskHandle_t Wifi_Task = NULL;
TaskHandle_t CMD_Task = NULL;
QueueHandle_t received_queue = NULL;

// Main loop
int main()
{
    stdio_init_all();
    cyw43_arch_init();
    setup_buttons();
    setup_display();
    setup_sensor();

    received_queue = xQueueCreate(10, 10);

    xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
    xTaskCreate(display_task, "Display_Task", 4096, NULL, 1, NULL);
    xTaskCreate(accelerometer_task, "Accel_Task", 4096, NULL, 1, NULL);
    xTaskCreate(wifi_task, "Wifi_Task", 4096, NULL, 1, &Wifi_Task);
    xTaskCreate(message_handler, "MSG_Task", 2048, NULL, 1, NULL);
    xTaskCreate(send_commands, "CMD_Task", 2048, NULL, 1, &CMD_Task);
    vTaskStartScheduler();

    while(1){};
}