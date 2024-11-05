#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "barcodes.h"
#include "queue.h"

/*Stores processed width as 0 and 1 for narrow and wide respectively*/
uint16_t barcode_lists = 0;
bool ready_to_decode = false;

void print_binary(uint16_t value) {
    for (int i = 15; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
    }
    printf("\n");
}

void barcode_width_processor() {
    float pulse = 0;
    int count = 0;
    int total_count = 0;

    vTaskDelay(pdMS_TO_TICKS(2500));
    printf("Barcode Width Task Started\n");

    while(1) {

        // Print the current binary representation after each pulse processing
        
        print_binary(barcode_lists);

        if (xSemaphoreTake(Barcode_BinarySemaphore, portMAX_DELAY));

        total_count += 1;

        pulse = IR_pulse_width_BAR;
        
        if (pulse <= 10){
            
        } else{

            count += 1;
            printf("Pulse: %f Good: %d Total: %d\n", pulse, count, total_count);

            // Shift existing bits to the left by 1 to make space for the new bit
            barcode_lists <<= 1;

            // Classify the pulse as narrow or wide and set the new bit accordingly
            if (pulse <= THRESHOLD) {
                
                // Narrow pulse: set the rightmost bit to 0 (no action needed since it's already 0)

            } else {
                // Wide pulse: set the rightmost bit to 1
                barcode_lists |= 1;
            }

        }
    }
}