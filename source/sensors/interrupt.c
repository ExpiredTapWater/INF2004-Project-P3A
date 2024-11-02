#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "sensors.h"

void interrupt_dispatcher(uint gpio, uint32_t events) {

    if (gpio == LEFT_ENCODER || gpio == RIGHT_ENCODER) {
        encoder_callback(gpio, events);
    } else{
        ultrasonic_callback(gpio, events);
    }

}

void setup_interrupts(){

    // Initialize the wheel encoder pins
    gpio_init(LEFT_ENCODER);
    gpio_init(RIGHT_ENCODER);
    gpio_set_dir(LEFT_ENCODER, GPIO_IN);
    gpio_set_dir(RIGHT_ENCODER, GPIO_IN);
    gpio_pull_up(LEFT_ENCODER);
    gpio_pull_up(RIGHT_ENCODER);

    // Set up interrupts to trigger only on the rising edge
    gpio_set_irq_enabled_with_callback(LEFT_ENCODER, GPIO_IRQ_EDGE_RISE, true, &interrupt_dispatcher);
    gpio_set_irq_enabled(RIGHT_ENCODER, GPIO_IRQ_EDGE_RISE, true);

    // Ultrasonic Stuff
    gpio_init(TRIG_PIN);
    gpio_init(ECHO_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
    
    // Remember that echo needs to catch both rising and falling
    gpio_set_irq_enabled(ECHO_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

}