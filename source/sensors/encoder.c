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

void encoder_callback(uint gpio, uint32_t events) {

    uint32_t current_time = time_us_32();

    if (gpio == LEFT_ENCODER) {
        if (last_pulse_time_L == 0) {
            last_pulse_time_L = current_time;
            return;
        }

        pulse_width_L = current_time - last_pulse_time_L;
        last_pulse_time_L = current_time;

    } else if (gpio == RIGHT_ENCODER) {
        if (last_pulse_time_R == 0) {
            last_pulse_time_R = current_time;
            return;
        }

        pulse_width_R = current_time - last_pulse_time_R;
        last_pulse_time_R = current_time;
    }
}

void reset_encoder() {

    // Reset encoder variables
    pulse_width_L = 0;
    pulse_width_R = 0;
    last_pulse_time_L = 0;
    last_pulse_time_R = 0;
    
}

void setup_encoder_old(){
    // Initialize the wheel encoder pins
    gpio_init(LEFT_ENCODER);
    gpio_init(RIGHT_ENCODER);
    gpio_set_dir(LEFT_ENCODER, GPIO_IN);
    gpio_set_dir(RIGHT_ENCODER, GPIO_IN);
    gpio_pull_up(LEFT_ENCODER);
    gpio_pull_up(RIGHT_ENCODER);

    // Set up interrupts to trigger only on the rising edge
    gpio_set_irq_enabled_with_callback(LEFT_ENCODER, GPIO_IRQ_EDGE_RISE, true, &encoder_callback);
    gpio_set_irq_enabled(RIGHT_ENCODER, GPIO_IRQ_EDGE_RISE, true);

}