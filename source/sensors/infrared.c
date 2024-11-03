#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "sensors.h"

volatile uint32_t IR_last_pulse_time_L = 0;
volatile uint32_t IR_pulse_width_L = 0;
volatile bool IR_state_L = false;

volatile uint32_t IR_last_pulse_time_R = 0;
volatile uint32_t IR_pulse_width_R = 0;
volatile bool IR_state_R = false;

volatile uint32_t IR_last_pulse_time_BAR = 0;
volatile uint32_t IR_pulse_width_BAR = 0;
volatile bool IR_state_BAR = false;

void ir_callback_L(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();

    if (IR_state_L == false) { // Rising edge
        IR_last_pulse_time_L = current_time;
        IR_state_L = true;
    } else { // Falling edge
        IR_pulse_width_L = current_time - IR_last_pulse_time_L;
        IR_state_L = false;
    }
}

void ir_callback_R(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();

    if (IR_state_R == false) { // Rising edge
        IR_last_pulse_time_R = current_time;
        IR_state_R = true;
    } else { // Falling edge
        IR_pulse_width_R = current_time - IR_last_pulse_time_R;
        IR_state_R = false;
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
    }
}


uint32_t get_IR_pulse_width_L(void) {
    uint32_t width;
    uint32_t state = save_and_disable_interrupts();
    width = IR_pulse_width_L;
    IR_pulse_width_L = 0;
    restore_interrupts(state);
    return width;
}

uint32_t get_IR_pulse_width_R(void) {
    uint32_t width;
    uint32_t state = save_and_disable_interrupts();
    width = IR_pulse_width_R;
    IR_pulse_width_R = 0;
    restore_interrupts(state);
    return width;
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

    uint32_t width_L = 0;
    uint32_t width_R = 0;
    uint32_t width_BAR = 0;

    while(1){

        uint32_t width_L = get_IR_pulse_width_L();
        uint32_t width_R = get_IR_pulse_width_R();
        uint32_t width_BAR = get_IR_pulse_width_BAR();

        printf("IR: %u-%u-%u\n", width_L, width_R, width_BAR);
        vTaskDelay(pdMS_TO_TICKS(1000));

    }
}