#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "timers.h"
#include "queue.h"

#define BUTTON_LEFT 22
#define BUTTON_CENTRE 21
#define BUTTON_RIGHT 20
#define DEBOUNCE_DELAY 250
#define MESSAGE_BUFFER 12

// From main.c
extern TaskHandle_t Wifi_Task;
extern TaskHandle_t CMD_Task;
extern xQueueHandle received_queue;

// FSM Try out. Depending on the state, will send xTaskNotify to the corresponding task.
typedef enum {
    BOOT = 0,
    WIFI,
    MAIN
} program_state_t;

volatile program_state_t current_state = BOOT;
volatile bool connection_cursor = false;
volatile bool heartbeat = false;
volatile bool disable = false;
volatile bool line_following = false;
volatile bool auto_line_following = false;

bool debounce_flag = false;
TimerHandle_t debounce_timer;

char barcode_slot_1[8] = "-";
char barcode_slot_2[8] = "-";
char barcode_slot_3[8] = "-";
char barcode_slot_4[8] = "-";
uint8_t barcode_counter = 0;

// From wifi.c
extern ip_addr_t car_ip;
extern volatile bool handshake_state;
extern volatile uint16_t total_packets_sent;
extern volatile uint16_t total_packets_received;
void send_udp_byte_packet(uint8_t data, const ip_addr_t *client_ip, uint16_t client_port);

// From display.c
extern volatile bool cursor;

// From sensor.c
extern volatile uint8_t mappings;
void set_neutral();

// Simple callback funciton to reset the debounce flag
void reset_debounce(TimerHandle_t xTimer){
    debounce_flag = false;
}

// Main ISR for main button. Determines what each button does per state
void centre_button_callback(uint gpio, uint32_t events){

    // If debouce flag has not been triggered. Ie this is first instance of button press
    if (!debounce_flag){

        // Set debounce flag
        debounce_flag = true;

        // Resets timer (Or starts it if never called)
        xTimerReset(debounce_timer, 0);

        switch (current_state) {

            case BOOT:
                current_state = WIFI; // Move to next state
                xTaskNotifyGive(Wifi_Task);
                break;

            case WIFI:
                if(handshake_state){
                    xTaskNotifyGive(CMD_Task);
                    current_state = MAIN;
                }
                break;
                
            case MAIN:
                if(!line_following){
                    disable = !disable;
                    if(disable){
                        send_udp_byte_packet(0x00, &car_ip, 2004);
                    }
                }
                break;
 
            default:
                printf("Unknown State\n");
                break;
        }
    }
}

// Main ISR for left button. Determines what each button does per state
void left_button_callback(uint gpio, uint32_t events){

    // If debouce flag has not been triggered. Ie this is first instance of button press
    if (!debounce_flag){

        // Set debounce flag
        debounce_flag = true;

        // Resets timer (Or starts it if never called)
        xTimerReset(debounce_timer, 0);

        switch (current_state) {

            case BOOT:
                connection_cursor = !connection_cursor; // Flip state
                break;

            case WIFI:
                break;

            case MAIN:
                // Switch to manual remote control
                line_following = false;
                auto_line_following = false;
                disable = false;
                send_udp_byte_packet(0xF1, &car_ip, 2004);
                break;
 
            default:
                printf("Unknown State\n");
                break;
        }
    }
}

// Main ISR for left button. Determines what each button does per state
void right_button_callback(uint gpio, uint32_t events){

    // If debouce flag has not been triggered. Ie this is first instance of button press
    if (!debounce_flag){

        // Set debounce flag
        debounce_flag = true;

        // Resets timer (Or starts it if never called)
        xTimerReset(debounce_timer, 0);

        switch (current_state) {

            case BOOT:
                connection_cursor = !connection_cursor; // Flip state
                break;

            case WIFI:
                break;

            case MAIN:
                // Switch to auto handover line following
                line_following = false;
                auto_line_following = true;
                disable = false;
                send_udp_byte_packet(0xF4, &car_ip, 2004);
                break;
 
            default:
                printf("Unknown State\n");
                break;
        }
    }
}

void change_state(uint8_t state){
    current_state = state;
}

// Sends interrupts to the correct callbacks
void interrupt_dispatcher(uint gpio, uint32_t events) {
    
    // Organised in ascending pins, most likely to least likely
    switch (gpio) {
        case BUTTON_LEFT:
            left_button_callback(gpio, events);
            break;
        case BUTTON_CENTRE:
            centre_button_callback(gpio, events);
            break;
        case BUTTON_RIGHT:
            right_button_callback(gpio, events);
            break;
        default:
            break;
    }
}

// Setup the button and attache them to ISR
void setup_buttons(){

    // Setup hardware
    gpio_init(BUTTON_CENTRE);
    gpio_init(BUTTON_LEFT);
    gpio_init(BUTTON_RIGHT);
    gpio_set_dir(BUTTON_CENTRE, GPIO_IN);
    gpio_set_dir(BUTTON_LEFT, GPIO_IN);
    gpio_set_dir(BUTTON_RIGHT, GPIO_IN);
    gpio_pull_up(BUTTON_CENTRE);
    gpio_pull_up(BUTTON_LEFT);
    gpio_pull_up(BUTTON_RIGHT);

    // Attach interrupts
    gpio_set_irq_enabled_with_callback(BUTTON_CENTRE, GPIO_IRQ_EDGE_RISE, true, &interrupt_dispatcher);
    gpio_set_irq_enabled(BUTTON_LEFT, GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(BUTTON_RIGHT, GPIO_IRQ_EDGE_RISE, true);

    // Create timer to reset debounce flag
    debounce_timer = xTimerCreate("Debounce",pdMS_TO_TICKS(DEBOUNCE_DELAY),pdFALSE,NULL,reset_debounce);
}

void barcode_task(void *pvParameters){

    
}


void message_handler(void *pvParameters){

    char message[MESSAGE_BUFFER] = {0};
    char temp[8] = {0};

    while(1){

        while(handshake_state){
            // Wait for message
            xQueueReceive(received_queue, &message, portMAX_DELAY);
            heartbeat = !heartbeat;

            if(message[0] == '-'){ // Barcode

                // Formats string to be displayed on screen
                barcode_counter += 1;
                strcpy(barcode_slot_4, barcode_slot_3);
                strcpy(barcode_slot_3, barcode_slot_2);
                strcpy(barcode_slot_2, barcode_slot_1);

                // +1 skips the initial "-" prefix for barcode messages
                snprintf(temp, 8, "%d: %s", barcode_counter, message + 1);
                strcpy(barcode_slot_1, temp);
            
            } else if (message[0] == '+'){
                disable = true;
            }
            
            else{ // Heartbeat
                total_packets_received =  atoi(message);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void send_commands(void *pvParameters){

    static bool spam_prevention = false;

    // Wait for everything to be setup before trying to send commands
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    
    while(1){
        if (!disable){
            if(mappings == 0){

                // Prevents repeated sending of stop commands
                if (!spam_prevention){
                    spam_prevention = true;
                    send_udp_byte_packet(0x00, &car_ip, 2004);
                    vTaskDelay(pdMS_TO_TICKS(1));
                    send_udp_byte_packet(0x00, &car_ip, 2004);
                    vTaskDelay(pdMS_TO_TICKS(1));
                    send_udp_byte_packet(0x00, &car_ip, 2004);
                    total_packets_sent += 3;
                }
            }else{
                spam_prevention = false;
                send_udp_byte_packet(mappings, &car_ip, 2004);
                total_packets_sent += 1;
            }
            
        }
        vTaskDelay(pdMS_TO_TICKS(100)); //10Hz send rate.
    }
}