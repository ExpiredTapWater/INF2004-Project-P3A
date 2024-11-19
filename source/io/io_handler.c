#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "timers.h"
#include "queue.h"

#define BUTTON_LEFT 22
#define BUTTON_CENTRE 21
#define BUTTON_RIGHT 20
#define DEBOUNCE_DELAY 250
#define MESSAGE_BUFFER 16

extern xQueueHandle received_queue;

// From wifi.c
extern volatile uint16_t total_packets_sent;
extern volatile uint16_t total_packets_received;
void send_udp_byte_packet(uint8_t data, const ip_addr_t *client_ip, uint16_t client_port);

void message_handler(void *pvParameters) {

    char message[MESSAGE_BUFFER] = {0};
    float distance = 0.0, speed = 0.0, sensor = 0.0;

    while (1) {

        // Wait for message
        xQueueReceive(received_queue, &message, portMAX_DELAY);

        // Parse the message formatted as x-y-z
        if (sscanf(message, "%f-%f-%f", &distance, &speed, &sensor) == 3) {

            printf("Distance: %.2fm, Speed: %.2fm/s, Ultrasonic: %.2fcm\n", distance, speed, sensor);

        } else {

            printf("Error: Invalid message format: %s\n", message);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}