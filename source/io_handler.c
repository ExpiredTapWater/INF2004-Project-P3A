// Pico
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include <queue.h>
// Our Headers
#include "header.h"

bool HANDSHAKE = false; // Remote connected state
//bool IN_MOTION = false; // Car moving state

void heartbeat_task(void *pvParameters){
    while(1){
        if(HANDSHAKE){
            char pckt[10];
            sprintf(pckt, "%d", total_packets_received);
            send_udp_packet(pckt, &remote_ip, 2004);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void reply_handshake(char* message){
    if (strcmp(message, "ESP-Hello") == 0){
        send_udp_packet("Pico-Hello", &remote_ip, 2004);
        total_packets_received -= 1; // Remove handshake packet count
        HANDSHAKE = true;
    }
}

int parse_command(char* command){

    // Stop command is sent
    if (strcmp(command, "0000") == 0) { // Stop
        return 0;

    } else{  // Check commands in priotiry of Front>Back>Left>Right

        if (command[2] == '1'){
            return 1; //Forward
        } else if (command[2] == '2'){
            return 5; //Forward-2
        } else if (command[3] >= '1'){
            return 2; //Reverse
        } else if (command[0] >= '1'){
            return 3; //Left
        } else if (command[1] >= '1'){
            return 4; //Right
        }{
            return 9;
        }
    }
}

void message_handler(void *pvParameters){
    char message[MESSAGE_BUFFER] = {0};
    char previous_command[10] = {0};
    int motor_command = 0;

    while(1){
        led_off();
        xQueueReceive(received_queue, message, portMAX_DELAY);
        led_on();
        //printf("main.c: %s total: %d\n", message, total_packets_received);

        // No remote has been connected
        if (!HANDSHAKE){
            reply_handshake(message);
        
        // Check for any other forms of commands like MANUAL/AUTO here before assuming its a motor cmd

        } else {

            // If latest command received matches the previus command, ignore
            if (strcmp(message, previous_command) != 0) {
                strncpy(previous_command, message, 10);

                // Parse message to get motor command, then send to queue
                motor_command = parse_command(message);
                if (xQueueSend(commands_queue, &motor_command, 0U) != pdPASS) {
                    printf("Command Queue is full!\n");
                }

            }
        }
    }
}

// Quickly read the state of a GPIO pin
bool get_connection_mode(void){

    gpio_init(MODE_SELECT_BUTTON);
    gpio_set_dir(MODE_SELECT_BUTTON, GPIO_IN);
    gpio_pull_up(MODE_SELECT_BUTTON);

    bool state = gpio_get(MODE_SELECT_BUTTON);
    return state;
}