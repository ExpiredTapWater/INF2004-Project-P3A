#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "FreeRTOS.h"
#include "task.h"
#include <queue.h>
#include "header.h"

// Flags
bool BUZZER_INIT = false;
bool HANDSHAKE = false; // Remote connected state

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

void reply_handshake(uint8_t message){
    if (message == (uint8_t) 0xFF){
        send_udp_packet("Pico-Hello", &remote_ip, 2004);
        total_packets_received -= 1; // Remove handshake packet count
        HANDSHAKE = true;
        buzzer(2);
    }
}

int parse_command(uint8_t command){

    // Stop command is sent
    if (command == 0) { // Stop
        return 0;

    } else{  // Check commands in priotiry of Front>Back>Left>Right

        if (command == '1'){
            return 1; //Forward
        } else if (command == '2'){
            return 5; //Forward-2
        } else if (command >= '3'){
            return 2; //Reverse
        } else if (command >= '4'){
            return 3; //Left
        } else if (command >= '5'){
            return 4; //Right
        }{
            return 9;
        }
    }
}

void message_handler(void *pvParameters){
    uint8_t message = 1;
    uint8_t previous_command = 1;

    while(1){
        led_off();
        xQueueReceive(received_queue, &message, portMAX_DELAY);
        led_on();
        //printf("main.c: %s total: %d\n", message, total_packets_received);

        // No remote has been connected
        if (!HANDSHAKE){
            reply_handshake(message);
        
        // Remote has already been connected
        } else {

            // Check for special command messsages
            if (message == 0xF1 || message == 0xF2) {
                buzzer(1);

                // Notify the mode_switcher_task of the mode change
                xTaskNotify(TaskManager_T, message, eSetValueWithOverwrite);
            }
            
            // Else the message should be a motor command, so send over to core_1
            else if (previous_command != message){
                
                // If latest command received matches the previus command, ignore
                previous_command = message;

                // Directly send message to command_queue
                if (xQueueSend(commands_queue, &message, 0U) != pdPASS) {
                    printf("Command Queue is full!\n"); // You should never see this, hopefully
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

// Auxilary Function
void buzzer(int count){

    if (!BUZZER_INIT){
        
        gpio_init(BUZZER_PIN);
        gpio_set_dir(BUZZER_PIN, GPIO_OUT);

        uint32_t divider = CLOCK_FREQUENCY / (FREQUENCY * 65536);

        gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
        uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
        pwm_set_clkdiv(slice, divider);
        pwm_set_wrap(slice, 65535);
        pwm_set_enabled(slice, true);

        BUZZER_INIT = true;

    }

    for (int i = 0; i < count; i++) {

        pwm_set_gpio_level(BUZZER_PIN, 65535 / 2);
        vTaskDelay(pdMS_TO_TICKS(200));
        pwm_set_gpio_level(BUZZER_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(200));

    }
}

void task_manager(void *pvParameters) {

    /* Mode Selection
    [0] = Mannual
    [1] = Auto (Line Following)
    [2] = ???? */
    uint32_t MODE = 0;

    while (1) {

        // Wait for a notification (more efficient than polling)
        xTaskNotifyWait(0x00, ULONG_MAX, &MODE, portMAX_DELAY);

        if (MODE == 0xF1) {
            // Switch to Manual Mode
            vTaskResume(LED_T);
            vTaskSuspend(GPIO_T);
            printf("LED on GPIO off\n");
            
        } else if (MODE == 0xF2) {
            // Switch to Auto Mode
            vTaskResume(GPIO_T);
            vTaskSuspend(LED_T);
            printf("LED off GPIO on\n");
        }
    }
}