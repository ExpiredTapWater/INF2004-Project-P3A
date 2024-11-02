#include "queue.h"
#include "lwip/ip_addr.h"

#define MESSAGE_BUFFER 16
#define MODE_SELECT_BUTTON 22

// main.c
extern QueueHandle_t received_queue;
extern QueueHandle_t commands_queue;

// blink.c
void led_on();
void led_off();
void flash(int count, bool mode);
void blink(void *pvParameters);
void GPIO_blink(void *param);

// wifi.c
extern uint16_t total_packets_received;
extern ip_addr_t remote_ip;
extern ip_addr_t telemetry_ip;
void start_UDP_server_ap(void *pvParameters);
void start_UDP_server_hotspot(void *pvParameters);
void send_udp_packet(const char *data, const ip_addr_t *client_ip, uint16_t client_port);

// io_handler.c
bool get_connection_mode(void);
void message_handler(void *pvParameters);
void heartbeat_task(void *pvParameters);

// encoder.c
#define LEFT_ENCODER 0
#define RIGHT_ENCODER 1

void setup_encoder();

// motor.c
void setup_gpio_motor();
void setup_pwm_motor(); 
void motor_task(void *params);
void process_motor_commands(void *params);
void encoder_debug_task(void *params);
