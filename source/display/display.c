#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "display/ssd1306.h"
#include "networking/wifi.h"

#define SDA_PIN 4
#define SCL_PIN 5
#define REFRESH_DELAY 50

#define CURSOR_CENTRE_X 27
#define CURSOR_CENTRE_Y 25
#define CURSOR_SMALL_OFFSET 10
#define CURSOR_LARGE_OFFSET 20

// From io_handler.c
extern volatile program_state_t current_state;
extern volatile bool connection_cursor; // True is hotspot
extern volatile uint16_t total_packets_sent;
extern volatile uint16_t total_packets_received;
extern volatile bool heartbeat;
extern volatile bool disable;
extern volatile bool line_following;
extern volatile bool auto_line_following;
extern char barcode_slot_1[8];
extern char barcode_slot_2[8];
extern char barcode_slot_3[8];
extern char barcode_slot_4[8];
extern uint8_t barcode_counter;

// From sensor.c
extern volatile uint8_t mappings;

// From wifi.c
extern volatile bool handshake_state;

ssd1306_t disp;

void setup_display(){

    // Display
    i2c_init(i2c0, 400000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    disp.external_vcc=false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c0);

}

void display_boot(){

    static uint8_t x, y;

    // Clear display
    ssd1306_clear(&disp);

    // Header Text
    ssd1306_draw_string(&disp, 12, 2, 1, "Network Selection");

    // Content
    ssd1306_draw_string(&disp, 4, 18, 1, "HOTSPOT");
    ssd1306_draw_string(&disp, 4, 28, 1, HOTSPOT_IP_STRING);
    ssd1306_draw_string(&disp, 4, 43, 1, "ACCESS-POINT");
    ssd1306_draw_string(&disp, 4, 53, 1, AP_IP_STRING);

    // Set cursor position
    if(connection_cursor){

        // Selected hotspot
        x = 14;
        y = 38;

    }else{ 

        // Selected AP
        x = 39;
        y = 63;
    }

    // Draw cursor
    ssd1306_draw_line(&disp, 0, x, 127, x);
    ssd1306_draw_line(&disp, 0, y, 127, y);
    ssd1306_draw_line(&disp, 0, x, 0,   y);
    ssd1306_draw_line(&disp, 127, x, 127, y);

    // Display
    ssd1306_show(&disp);
}

void display_wifi(){

    static uint8_t x, y;

    // Clear display
    ssd1306_clear(&disp);

    // Header Text
    if (!handshake_state){
        ssd1306_draw_string(&disp, 10, 2, 1, "Connecting To Pico");
    }else{
        ssd1306_draw_string(&disp, 34, 2, 1, "Connected!");
    }
    

    // Draw selected option
    if(connection_cursor){

        // Selected hotspot
        x = 14;
        y = 38;
        ssd1306_draw_string(&disp, 4, 18, 1, "HOTSPOT");
        ssd1306_draw_string(&disp, 4, 28, 1, HOTSPOT_IP_STRING);

        if (handshake_state){
            ssd1306_draw_string(&disp, 8, 50, 1, "Press GP21 to start");
        }

    }else{ 

        // Selected AP
        x = 39;
        y = 63;
        ssd1306_draw_string(&disp, 4, 43, 1, "ACCESS-POINT");
        ssd1306_draw_string(&disp, 4, 53, 1, AP_IP_STRING);

        if (handshake_state){
            ssd1306_draw_string(&disp, 8, 20, 1, "Press GP21 to start");
        }
    }

    // Draw cursor
    ssd1306_draw_line(&disp, 0, x, 127, x);
    ssd1306_draw_line(&disp, 0, y, 127, y);
    ssd1306_draw_line(&disp, 0, x, 0,   y);
    ssd1306_draw_line(&disp, 127, x, 127, y);

    // Display
    ssd1306_show(&disp);
}

void display_main_ui(){

    static uint8_t x, y;
    static char sent_buffer[8];
    static char recv_buffer[8];

    // Clear display
    ssd1306_clear(&disp);

    // Draw Box
    ssd1306_draw_line(&disp, 0, 0, 63, 0);
    ssd1306_draw_line(&disp, 0, 63, 63, 63);
    ssd1306_draw_line(&disp, 0, 0, 0, 63);
    ssd1306_draw_line(&disp, 63, 0, 63, 63);

    // Check if sending packets is temporary disabled
    if(disable){

        if(line_following){
            ssd1306_draw_string(&disp, 9, 19, 2, "LINE");
            ssd1306_draw_string(&disp, 6, 37, 1, "FOLLOWING");

        }
        
        else{

            // Draw cross
            ssd1306_draw_line(&disp, 0, 0, 63, 63);
            ssd1306_draw_line(&disp, 0, 63, 63, 0);
        }

    }else{

        // Calculate cursor position
        x = CURSOR_CENTRE_X; // Set to neutral position first
        y = CURSOR_CENTRE_Y; // Offset slightly up-left

        // Apply mask and check if leftmost 4 bits are zero meaning neutral
        if ((mappings & 0xF0) == 0){ // Mask: 11110000
            ssd1306_draw_string(&disp, x, y, 2, "+"); // Draw cross in the centre

        // Calcuate offset based on direction and magnitude
        }else{

            // Check for forward
            if ((mappings & 0x80) != 0){ // Mask: 10000000

                // Check magnitude
                if ((mappings & 0x08) == 0){ // Low
                    y = CURSOR_CENTRE_Y - CURSOR_SMALL_OFFSET;
                }else{  // High
                    y = CURSOR_CENTRE_Y - CURSOR_LARGE_OFFSET;
                }
            }

            // Check for backward
            if ((mappings & 0x40) != 0){ // Mask: 01000000
                        
                // Check magnitude
                if ((mappings & 0x04) == 0){ // Low
                    y = CURSOR_CENTRE_Y + CURSOR_SMALL_OFFSET;
                }else{  // High
                    y = CURSOR_CENTRE_Y + CURSOR_LARGE_OFFSET;
                }

            }

            // Check for leftwards
            if ((mappings & 0x20) != 0){ // Mask: 00100000

                // Check magnitude
                if ((mappings & 0x02) == 0){ // Low
                    x = CURSOR_CENTRE_X - CURSOR_SMALL_OFFSET;
                }else{  // High
                    x = CURSOR_CENTRE_X - CURSOR_LARGE_OFFSET;
                }

            }

            // Check for rightwards
            if ((mappings & 0x10) != 0){ // Mask: 00010000

                // Check magnitude
                if ((mappings & 0x01) == 0){ // Low
                    x = CURSOR_CENTRE_X + CURSOR_SMALL_OFFSET;
                }else{  // High
                    x = CURSOR_CENTRE_X + CURSOR_LARGE_OFFSET;
                }
                
            }

            // Draw cursor using new x and y values
            ssd1306_draw_string(&disp, x, y, 2, "+");

        }

    }

    // Show packets sent/received
    sprintf(sent_buffer, "%d", total_packets_sent);
    sprintf(recv_buffer, "%d", total_packets_received);

    ssd1306_draw_string(&disp, 68, 2, 1, "S:");
    ssd1306_draw_string(&disp, 80, 2, 1, sent_buffer);
    ssd1306_draw_string(&disp, 68, 12, 1, "R:");
    ssd1306_draw_string(&disp, 80, 12, 1, recv_buffer);

    ssd1306_draw_line(&disp, 67, 22, 122, 22);

    // Show last 4 messages
    ssd1306_draw_string(&disp, 68, 26, 1, barcode_slot_1);
    ssd1306_draw_string(&disp, 68, 36, 1, barcode_slot_2);
    ssd1306_draw_string(&disp, 68, 46, 1, barcode_slot_3);
    ssd1306_draw_string(&disp, 68, 56, 1, barcode_slot_4);

    // Show heartbeat
    if (heartbeat){
        ssd1306_draw_string(&disp, 122, 1, 1, "o");
    }

    if (auto_line_following){
        ssd1306_draw_string(&disp, 3, 2, 1, "A");
    }
    
    // Display
    ssd1306_show(&disp);
}

void display_task(){

    while(1){

        // Display UI corresponding to the state
        switch (current_state) {

            case BOOT:
                display_boot();
                break;

            case WIFI:
                display_wifi();
                break;
                
            case MAIN:
                display_main_ui();
                break;

            default:
                printf("Unknown State\n");
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY));

    }

}