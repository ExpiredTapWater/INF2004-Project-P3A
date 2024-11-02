#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "sensors.h"
#include "semphr.h"

volatile uint32_t pulse_start = 0;
volatile uint32_t pulse_end = 0;
SemaphoreHandle_t Ultrasonic_BinarySempahore;

// Gets called from interrupt_dispatcher from interrupt.c
void ultrasonic_callback(uint gpio, uint32_t events){

    if (events & GPIO_IRQ_EDGE_RISE) {
        // Rising edge detected: Start timing
        pulse_start = time_us_32(); // Record start time in microseconds

    } else {
        // Falling edge detected
        pulse_end = time_us_32();

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        // Release semaphore now that falling edge has been detected (ie. ready to calculate)
        xSemaphoreGiveFromISR(Ultrasonic_BinarySempahore, &xHigherPriorityTaskWoken);

        // Yield to a higher prioriy task after completion (if there is, currently does nothing)
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

}

// Function to measure distance with HC-SR04
float get_distance() {

    // Clear the semaphore before starting
    xSemaphoreTake(Ultrasonic_BinarySempahore, 0);

    // Send trigger pulse
    gpio_put(TRIG_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(0.01));  // Wait for 10 ms
    gpio_put(TRIG_PIN, 0);

    // Wait for measurement to complete
    if (xSemaphoreTake(Ultrasonic_BinarySempahore, pdMS_TO_TICKS(TIMEOUT)) == pdTRUE) {
        // Measurement complete
        uint32_t pulse_duration = pulse_end - pulse_start;
        float distance = (pulse_duration / 58.0f);  // Conversion factor to get cm

        // Transform with predefined calibration values
        distance = (distance*CALIBRATION_SCALING) + CALIBRATION_OFFSET;
        return distance;
        
    } else {
        // Timeout or error
        return 999.0f;  // Indicate error
    }
}

void sample_ultrasonic_task(void *pvParameters){

    Ultrasonic_BinarySempahore = xSemaphoreCreateBinary();

    vTaskDelay(pdMS_TO_TICKS(3000));
    printf("Starting Ultrasonic Task");
    float mydistance = 0.0f;

    while(1){
        mydistance = get_distance();
        printf("Distance: %.4f\n", mydistance);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}