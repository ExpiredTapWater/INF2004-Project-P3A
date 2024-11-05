#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "sensors.h"
#include "semphr.h"

SemaphoreHandle_t Barcode_BinarySemaphore;

volatile bool black_detected = false;
volatile uint32_t IR_last_pulse_time_BAR = 0;
volatile uint32_t IR_pulse_width_BAR = 0;
volatile bool IR_state_BAR = false;

void ir_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_RISE) {
        black_detected = true; // Rising edge detected - black detected

    } else if (events & GPIO_IRQ_EDGE_FALL) {
        black_detected = false; // Falling edge detected - white detected
    }
}

void ir_callback_BAR(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();

    if (IR_state_BAR == false) { // Rising edge
        IR_last_pulse_time_BAR = current_time;
        IR_state_BAR = true;

    } else { // Falling edge
        IR_pulse_width_BAR = current_time - IR_last_pulse_time_BAR;
        IR_state_BAR = false;
        
        // Alert using semaphore
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        // Release semaphore now that falling edge has been detected (ie. ready to calculate)
        xSemaphoreGiveFromISR(Barcode_BinarySemaphore, &xHigherPriorityTaskWoken);

        // Yield to a higher prioriy task after completion (if there is, currently does nothing)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    }
}

void create_semaphores_barcode(){

    Barcode_BinarySemaphore = xSemaphoreCreateBinary();

    if (Barcode_BinarySemaphore == NULL) {
        while(1){
            printf("Failed to create Barcode_BinarySemaphore\n");
            sleep_ms(500);
        }
    }
}

uint32_t get_IR_pulse_width_BAR(void) {
    uint32_t width;
    uint32_t state = save_and_disable_interrupts();
    width = IR_pulse_width_BAR;
    IR_pulse_width_BAR = 0;
    restore_interrupts(state);
    return width;
}

void sample_ir_task(){

    uint32_t width_BAR = 0;

    while(1){

        uint32_t width_BAR = get_IR_pulse_width_BAR();

        printf("IR: %u\n", width_BAR);
        vTaskDelay(pdMS_TO_TICKS(1000));

    }
}