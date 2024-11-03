#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sensors.h"

volatile uint32_t pulse_width_L = 0;
volatile uint32_t pulse_width_R = 0;
volatile uint32_t last_pulse_time_L = 0;
volatile uint32_t last_pulse_time_R = 0;

void encoder_callback_L(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();

    if (last_pulse_time_L == 0) {
        last_pulse_time_L = current_time;
        return;
    }

    pulse_width_L = current_time - last_pulse_time_L;
    last_pulse_time_L = current_time;
}

void encoder_callback_R(uint gpio, uint32_t events) {
    uint32_t current_time = time_us_32();

    if (last_pulse_time_R == 0) {
        last_pulse_time_R = current_time;
        return;
    }

    pulse_width_R = current_time - last_pulse_time_R;
    last_pulse_time_R = current_time;
}

void reset_encoder() {

    // Reset encoder variables
    pulse_width_L = 0;
    pulse_width_R = 0;
    last_pulse_time_L = 0;
    last_pulse_time_R = 0;
    
}