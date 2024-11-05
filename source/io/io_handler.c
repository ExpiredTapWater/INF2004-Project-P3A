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
uint8_t CURRENT_MODE = 1;

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

                // Only pass motor commands to commands queue if the selected mode is remote control
                if (CURRENT_MODE == 1){

                    // Directly send message to command_queue
                    if (xQueueSend(commands_queue, &message, 0U) != pdPASS) {
                        printf("Command Queue is full!\n"); // You should never see this, hopefully
                    }

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
    [1] = Mannual
    [2] = Auto (Line Following)
    [3] = ???? */

    uint32_t REQUESTED_MODE = 0; // Can't use uint8 here

    while (1) {

        // Wait for a notification (more efficient than polling)
        xTaskNotifyWait(0x00, ULONG_MAX, &REQUESTED_MODE, portMAX_DELAY);

        switch (REQUESTED_MODE) {

            // Cast to uint8, upper bits not needed. Not sure if faster
            case (uint8_t)REMOTE_MODE:

                // Purge all pending commands
                xQueueReset(received_queue);
                xQueueReset(commands_queue);

                vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to ensure queue has been flushed

                // Disable line following function
                // Signal to line following function to stop
                xTaskNotifyGive(LineFollowing_T);

                // Resume motor command processing
                vTaskResume(Command_T);
                vTaskResume(Motor_T);

                vTaskDelay(pdMS_TO_TICKS(100)); // Small delay

                // Enable wheel encoders
                enable_encoder_interrupts();

                vTaskDelay(pdMS_TO_TICKS(50)); // Small delay

                // Update currently selected mode
                CURRENT_MODE = 1;
                printf("Remote Operation Enabled\n");

                // Purge all pending commands
                xQueueReset(received_queue);
                xQueueReset(commands_queue);

                // Add in a final motor stop command
                xQueueSend(commands_queue, 0x00, 0U);

                break;

            case (uint8_t)AUTOMATIC_MODE:

                // Purge all pending commands
                xQueueReset(received_queue);
                xQueueReset(commands_queue);

                // Add in a final motor stop command
                xQueueSend(commands_queue, 0x00, 0U);

                // Check that all movements has stopped
                while(pulse_width_L != 0 || pulse_width_R != 0){
                    vTaskDelay(pdMS_TO_TICKS(200)); // Give the motor time to complete the task
                }

                // Suspend motor command processing
                vTaskSuspend(Command_T);
                vTaskSuspend(Motor_T);

                // Disable wheel encoders
                disable_encoder_interrupts();

                vTaskDelay(pdMS_TO_TICKS(50)); // Small delay

                // Signal to line following function to proceed
                xTaskNotifyGive(LineFollowing_T);
                printf("Line Following Enabled\n");

                // Update current selected mode
                CURRENT_MODE = 2;

                // Reset queue again in case the driver keeps switches modes while the car is still moving
                vTaskDelay(pdMS_TO_TICKS(100));
                xQueueReset(received_queue);
                xQueueReset(commands_queue);

                break;

            default:
                break;
        }
    }
}