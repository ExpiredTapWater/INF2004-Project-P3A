#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "FreeRTOS.h"
#include "task.h"
#include <queue.h>
#include "semphr.h"

// External functions and variables
void update_motor(float speed_motor1, float speed_motor2, bool clockwise_motor1, bool clockwise_motor2);
void update_motor_fast(uint16_t speed_motor1, uint16_t speed_motor2);

// From infrared.c
extern volatile bool black_detected;

// From ultrasonic.c
extern SemaphoreHandle_t UltrasonicWarn_BinarySemaphore;

// ------- Define some simple motor macros -------

// Slowly speed up to overcome static friction
void ramp_up() {

    float motor1 = 0;
    float motor2 = 0;

    for (int i = 40; i <= 80; i += 2) {

        motor1 = i;
        motor2 = i+1;
        update_motor_fast(motor1, motor2);
        vTaskDelay(pdMS_TO_TICKS(20));
        
    }
}

void crawl(void){
    ramp_up();
    update_motor_fast(80, 80);
}

void forward(void){
    update_motor_fast(70, 73);
}
void turn_left(void){
    update_motor_fast(50, 70);
}

void turn_right(void){
    update_motor_fast(70, 50);
}

void stop(void){
    update_motor_fast(0, 0);
}

// Simple line following algorithm, setup for IR mounted on the left. Ie. we skirt along the left edge.
void line_following_task(void *pvParameters){


    /* Black magic underflow and overflow code. 
    MUST be uint8! Somehow the pattern of under and overflows allows the car to follow the line amazingly well.
    Don't ask me how, don't ask me why lol*/
    uint8_t left_momentum = 40;
    uint8_t right_momentum = 40;
    const uint8_t INCREMENT = 1;
    const uint8_t DECREMENT = 1;
    uint32_t start_command = false; // Needs to be int32 for task notifcation

    while(1){

        // Not sure if needed but won't hurt
        update_motor(0, 0, true, true);
        
        // Wait until task manager explicitly tells it to start. The same notification is used to kick the task out to idle.
        while (1) {
            if (xTaskNotifyWait(0x00, ULONG_MAX, &start_command, portMAX_DELAY) == pdPASS) {
                if (start_command) {
                    break;
                }
                // Notified but did not ask task to start, continue to wait.
            }
        }

        // Crawl forward till black line is reached
        forward();
        while(black_detected != 1){

                // Exit checking
                if (xTaskNotifyWait(0x00, ULONG_MAX, &start_command, 0) == pdPASS) {
                    update_motor_fast(0, 0);
                    break;
                }

            vTaskDelay(pdMS_TO_TICKS(4));

        }

        // Enter main loop
        while(1){

            // Check if ultrasonic gave any warnings. Reset speed to zero if so.
            if (xSemaphoreTake(UltrasonicWarn_BinarySemaphore, 0) == pdTRUE) {
                update_motor_fast(0, 0);
                printf("Distance Warning, ending line following\n");
                break;
            } 
    
            // When detected black, turn left
            if(black_detected == 1){
                update_motor_fast(left_momentum, 70);
                left_momentum += INCREMENT;
                right_momentum -= DECREMENT;

            } else{ // when detected white, turn right
                update_motor_fast(70, right_momentum);
                right_momentum += INCREMENT;
                left_momentum -= DECREMENT;
            }
            
            // Exit checking
            if (xTaskNotifyWait(0x00, ULONG_MAX, &start_command, 0) == pdPASS) {
                update_motor_fast(0, 0);
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(1));

        }
    }

}